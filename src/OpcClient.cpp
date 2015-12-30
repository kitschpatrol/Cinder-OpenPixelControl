// OpcClient.cpp
// CinderOpenPixelControlSandbox
//
// Created by Eric Mika on 12/24/15.

#include "OpcClient.h"

#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/Utilities.h"
#include "cinder/app/App.h"

#pragma mark Lifecycle

using namespace kp::opc;

ClientRef Client::create(std::string host, int32_t port, bool isLongConnection, bool isBandwidthConserved, ColorOrder colorOrder) {
	return ClientRef(new Client(host, port, isLongConnection, isBandwidthConserved, colorOrder))->shared_from_this();
}

Client::Client(std::string host, int32_t port, bool isLongConnection, bool isBandwidthConserved, ColorOrder colorOrder) {
	CI_LOG_V("OpcClient Created");
	mHost = host;
	mPort = port;
	mIsLongConnection = isLongConnection;
	mIsBandwidthConserved = isBandwidthConserved;
	mColorOrder = colorOrder;
	mNumMessagesSent = 0;

	mLedBuffer = ci::Buffer::create(HEADER_LENGTH);
	unsigned char *bufferData = static_cast<unsigned char *>(mLedBuffer->getData());
	bufferData[0] = 0; // Channel 0
	bufferData[1] = 0; // Command 0
	bufferData[2] = 0; // LED data length high byte
	bufferData[3] = 0; // LED data length low byte

	mLedsNeedUpdate = true;
	mTcpClient = TcpClient::create(ci::app::App::get()->io_service());

	mTcpClient->connectConnectEventHandler(&Client::onClientConnect, this);
	mTcpClient->connectErrorEventHandler(&Client::onClientError, this);
	mTcpClient->connectResolveEventHandler(&Client::onClientEndpointResolved, this);

	connect();
}

Client::~Client() {
	CI_LOG_V("OpcClient Destroyed");
}

#pragma mark Networking Client

void Client::connect() {
	// CI_LOG_V("Connect");
	mLedsNeedUpdate = true;

	if (mTcpSession) {
		if (mTcpSession->getSocket()->is_open()) {
			mTcpSession->close();
		}
	}
	mTcpClient->connect(mHost, (uint16_t)mPort);
}

#pragma mark Networking Client Callbacks

void Client::onClientConnect(TcpSessionRef session) {
	// CI_LOG_V("Client Connected, creating session");

	// Get the session from the argument and set callbacks.
	// Note that you can use lambdas.

	if (mTcpSession) {
		mTcpSession.reset();
	}

	mTcpSession = session;
	mTcpSession->connectCloseEventHandler(&Client::onSessionClose, this);
	mTcpSession->connectErrorEventHandler(&Client::onSessionError, this);
	mTcpSession->connectWriteEventHandler(&Client::onSessionWrite, this);
}

void Client::onClientEndpointResolved() {
	// CI_LOG_V("Endpoint resolved");
}

void Client::onClientError(std::string err, size_t bytesTransferred) {
	CI_LOG_E("Client Error" << (err.empty() ? "" : (": " + err)));

	//	// This is unpleasant...
	if (!err.empty() && (err == "Connection refused")) {
		// try to reconnect
		connect();
	}
}

#pragma mark Networking Session

void Client::write() {
	// This sample is meant to work with only one session at a time.
	if (mTcpSession && mTcpSession->getSocket()->is_open()) {
		// CI_LOG_V("Writing to: " + mHost + ":" + ci::toString(mPort));
		mTcpSession->write(mLedBuffer);
	}
}

#pragma mark Networking Session Callbacks

void Client::onSessionError(std::string err, size_t bytesTransferred) {
	CI_LOG_E("Session Error" << (err.empty() ? "" : (": " + err)));

	if (!err.empty() && (err == "Broken pipe")) {
		// try to reconnect
		connect();
	}
}

void Client::onSessionClose() {
	// CI_LOG_V("Session Closed");
}

void Client::onSessionWrite(size_t bytesTransferred) {
	// CI_LOG_V(ci::toString(bytesTransferred) + " bytes written");
	mNumMessagesSent++;
	if (!mIsLongConnection) {
		// Close the connection and re-open for the next write
		connect();
	}
}

#pragma mark API

void Client::setLED(int index, ci::Color8u color) {
	// See if we need to enlarge the buffer
	const int requiredBufferLength = ((index + 1) * BYTES_PER_LED) + HEADER_LENGTH;

	if (requiredBufferLength > mLedBuffer->getSize()) {
		mLedBuffer->resize(requiredBufferLength);

		// update length in header
		unsigned char *bufferData = static_cast<unsigned char *>(mLedBuffer->getData());
		const uint16_t ledDataLength = static_cast<uint16_t>(requiredBufferLength - HEADER_LENGTH);
		bufferData[2] = ledDataLength >> 8;		// LED data length high byte
		bufferData[3] = ledDataLength & 0xFF; // LED data length low byte
	}

	// Check for changes, only send a new message if needed (though mIsBandwidthConserved can overrule)
	const int ledOffset = HEADER_LENGTH + (index * BYTES_PER_LED);
	unsigned char *bufferData = static_cast<unsigned char *>(mLedBuffer->getData());

	const int *colorOrder = &(COLOR_ORDER_LOOKUP[mColorOrder][0]);

	// Check each channel for a difference and set if needed
	for (int i = 0; i < 3; i++) {
		if (bufferData[ledOffset + i] != color[colorOrder[i]]) {
			bufferData[ledOffset + i] = color[colorOrder[i]];
			mLedsNeedUpdate = true;
		}
	}
}

void Client::update() {
	// Send pixel state to the OPC server (if needed)
	if (!mIsBandwidthConserved || mLedsNeedUpdate) {
		write();
		mLedsNeedUpdate = false;
	}
}

unsigned long long Client::getNumMessagesSent() {
	return mNumMessagesSent;
}
