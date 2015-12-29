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

	parseBufferWithOffset(buffer, 0);

	// Read next message
	mTcpSession->read();
}

void Server::parseBufferWithOffset(ci::BufferRef buffer, int offset) {
	if (buffer->getSize() < HEADER_LENGTH) {
		CI_LOG_E("Received OPC message with length: " << buffer->getSize() << " Too short.");
	} else {
		const uint8_t *data = static_cast<const uint8_t *>(buffer->getData());
		const uint8_t channel = data[0 + offset];																	 // 0 is broadcast to all channels
		const uint8_t command = data[1 + offset];																	 // 0 is set pixel color
		const uint16_t ledDataLength = (data[2 + offset] << 8) | data[3 + offset]; //
		const int numLeds = ledDataLength / BYTES_PER_LED;
		const int expectedLength = HEADER_LENGTH + ledDataLength;
		int nextOffset = 0;

		// Decide if we need to parse this buffer in chunks
		if (buffer->getSize() - offset > expectedLength) {
			nextOffset = offset + expectedLength;
			CI_LOG_E("OPC message was larger than promised length. Will parse again with an offset. Expected length: " << static_cast<int>(expectedLength)
																																																								 << "\tReceived length: " << buffer->getSize());
		}

		// Validate packet
		if (buffer->getSize() - offset < expectedLength) {
			CI_LOG_E("OPC message too short. Expected length: " << static_cast<int>(expectedLength) << "\tReceived length: " << buffer->getSize());
		} else if (command != 0) {
			CI_LOG_E("OPC message has unsupported command type: " << static_cast<int>(command));
		} else if (numLeds > mNumLeds) {
			CI_LOG_E("OPC message data too long: " << numLeds);
		} else if (channel != 0) {
			CI_LOG_E("OPC message was for channel: " << static_cast<int>(channel) << " Cinder OPC Block does not currently support multiple channels.");
		} else {
			// Everything looks ok
			mNumMessagesReceived++;

			// CI_LOG_V("Channel: " << static_cast<int>(channel) << "\tCommand: " << static_cast<int>(command) << "\tLength: " << length);

			// Set the model from the data
			const int *colorOrder = &(COLOR_ORDER_LOOKUP[mColorOrder][0]);

			for (int i = 0; i < numLeds; i++) {
				const int ledIndex = HEADER_LENGTH + (i * BYTES_PER_LED) + offset;
				mChannels[channel][i][colorOrder[0]] = data[ledIndex];
				mChannels[channel][i][colorOrder[1]] = data[ledIndex + 1];
				mChannels[channel][i][colorOrder[2]] = data[ledIndex + 2];
			}
		}

		// Recurse if needed
		if (nextOffset != 0) {
			parseBufferWithOffset(buffer, nextOffset);
		}
	}
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
