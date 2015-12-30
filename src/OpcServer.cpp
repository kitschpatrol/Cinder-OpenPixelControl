// OPCServer.cpp
// CinderOpenPixelControlSandbox
//
// Created by Eric Mika on 12/23/15.

#include "OpcServer.h"

#include "CinderAsio.h"
#include "cinder/Cinder.h"
#include "cinder/Log.h"

#include "cinder/Utilities.h"
#include "cinder/app/App.h"

using namespace kp::opc;

#pragma mark Lifecycle

ServerRef Server::create(int numLeds, int32_t port, ColorOrder colorOrder) {
	return ServerRef(new Server(numLeds, port, colorOrder))->shared_from_this();
}

Server::Server(int numLeds, int32_t port, ColorOrder colorOrder) {
	CI_LOG_V("Client Created");
	mNumLeds = numLeds;
	mPort = port;
	mColorOrder = colorOrder;
	mNumMessagesReceived = 0;

	// Temp set size... preliminary channel support
	mChannels.resize(1);
	mChannels[0].resize(mNumLeds);

	mTcpServer = TcpServer::create(ci::app::App::get()->io_service());

	// Add callbacks to work with the server asynchronously.
	mTcpServer->connectAcceptEventHandler(&Server::onAccept, this);
	mTcpServer->connectCancelEventHandler(&Server::onCancel, this);
	mTcpServer->connectErrorEventHandler(&Server::onError, this);

	accept();
}

Server::~Server() {
	CI_LOG_V("Client Destroyed");

	// clean up queue
	mBufferQueue.clear();

	mTcpServer->cancel();
}

#pragma mark Networking

void Server::accept() {
	// CI_LOG_V("Accept");

	if (mTcpSession) {
		mTcpSession.reset();
	}

	// Start listening
	if (mTcpServer) {
		mTcpServer->accept(static_cast<uint16_t>(mPort));
		// CI_LOG_V("Listening on port: " << mPort);
	}
}

#pragma mark Networking Callbacks

void Server::onAccept(TcpSessionRef session) {
	// CI_LOG_V("Connected");

	// Get the session from the argument and set callbacks.
	mTcpSession = session;
	mTcpSession->connectCloseEventHandler(&Server::onClose, this);
	mTcpSession->connectErrorEventHandler(&Server::onError, this);
	mTcpSession->connectReadEventHandler(&Server::onRead, this);
	mTcpSession->connectReadCompleteEventHandler(&Server::onReadComplete, this);

	// Start reading data from the client.
	mTcpSession->read();
}

void Server::onRead(ci::BufferRef buffer) {
	// See http://openpixelcontrol.org for spec.
	// Note that support for multiple channels is not implemented.

	// Enqueue received message
	const uint8_t *data = static_cast<const uint8_t *>(buffer->getData());
	for (int i = 0; i < buffer->getSize(); i++) {
		mBufferQueue.push_back(data[i]);
	}

	// Parse if possible
	bool isMoreToParse = true;
	while (isMoreToParse) {
		isMoreToParse = parseQueue(mBufferQueue);
	}

	// Read next message
	mTcpSession->read();
}

bool Server::parseQueue(std::deque<uint8_t> &queue) {

	// Validate the queue
	if (queue.size() < HEADER_LENGTH) {
		// CI_LOG_E("Received OPC message with length: " << queue.size() << " Too short.");
		return false;
	}

	// Peek into the queue
	const uint8_t channel = queue[0];													 // 0 is broadcast to all channels
	const uint8_t command = queue[1];													 // 0 is set pixel color
	const uint16_t ledDataLength = (queue[2] << 8) | queue[3]; //
	const int numLeds = ledDataLength / BYTES_PER_LED;
	const int expectedLength = HEADER_LENGTH + ledDataLength;

	if (queue.size() < expectedLength) {
		// CI_LOG_E("OPC message too short. Expected length: " << static_cast<int>(expectedLength) << "\tReceived length: " << queue.size());
		return false;
	}

	if (command != 0) {
		CI_LOG_E("OPC message has unsupported command type: " << static_cast<int>(command));
		return false;
	}

	if (channel != 0) {
		CI_LOG_E("OPC message was for channel: " << static_cast<int>(channel) << " Cinder OPC Block does not currently support multiple channels.");
		return false;
	}

	// Everything looks ok, read the data off the front
	mNumMessagesReceived++;

	// CI_LOG_V("Channel: " << static_cast<int>(channel) << "\tCommand: " << static_cast<int>(command) << "\tLength: " << length);

	// Set the model from the data
	const int *colorOrder = &(COLOR_ORDER_LOOKUP[mColorOrder][0]);

	for (int i = 0; i < numLeds; i++) {
		const int ledIndex = HEADER_LENGTH + (i * BYTES_PER_LED);

		for (int k = 0; k < 3; k++) {
			mChannels[channel][i][colorOrder[k]] = queue[ledIndex + k];
		}
	}

	// Clear what we read from the queue
	queue.erase(queue.begin(), queue.begin() + expectedLength);

	return true;
}

void Server::onReadComplete() {
	// CI_LOG_V("On Read complete");
	accept();
}

void Server::onError(std::string err, size_t bytesTransferred) {
	CI_LOG_V("Error" + (err.empty() ? "" : (": " + err)););
}

void Server::onClose() {
	// CI_LOG_V("On Close");
}

void Server::onCancel() {
	// CI_LOG_V("On Cancel");
}

const std::vector<ci::Color8u> Server::getLeds() const {
	return mChannels[0];
}

const unsigned long Server::getNumMessagesReceived() {
	return mNumMessagesReceived;
}
