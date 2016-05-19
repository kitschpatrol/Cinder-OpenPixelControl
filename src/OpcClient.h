// OpcClient.h
// CinderOpenPixelControlSandbox
//
// Created by Eric Mika on 12/24/15.
//

#pragma once

#include "CinderAsio.h"
#include "cinder/Color.h"

#include "OpcConstants.h"
#include "TcpClient.h"

namespace kp {
namespace opc {

typedef std::shared_ptr<class Client> ClientRef;

class Client : public std::enable_shared_from_this<Client> {
public:
	static ClientRef create(std::string host = kp::opc::DEFAULT_IP, int32_t port = kp::opc::DEFAULT_PORT, bool isLongConnection = true,
													bool isBandwidthConserved = true, kp::opc::ColorOrder colorOrder = kp::opc::DEFAULT_COLOR_ORDER);
	~Client();
	void setLED(int index, ci::Color8u color);

	void update(); // syncs local model to LEDs
	unsigned long long getNumMessagesSent();

protected:
	Client(std::string host, int32_t port, bool isLongConnection, bool isBandwidthConserved, kp::opc::ColorOrder colorOrder);

private:
	TcpClientRef mTcpClient;
	TcpSessionRef mTcpSession;
	std::string mHost;
	int32_t mPort;
	bool mIsLongConnection;
	bool mIsBandwidthConserved;
	ci::BufferRef mLedBuffer;
	kp::opc::ColorOrder mColorOrder;
	unsigned long long mNumMessagesSent;

	bool mLedsNeedUpdate;

	void connect();
	void onClientConnect(TcpSessionRef session);
	void onClientEndpointResolved();
	void onClientError(std::string err, size_t bytesTransferred);

	void write();
	void onSessionClose();
	void onSessionError(std::string err, size_t bytesTransferred);
	void onSessionWrite(size_t bytesTransferred);
};

} // namespace opc
} // namespace kp