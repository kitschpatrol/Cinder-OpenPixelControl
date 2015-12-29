//
//  OPCServer.hpp
//  CinderOpenPixelControlSandbox
//
//  Created by Eric Mika on 12/23/15.
//
//

#pragma once

#include "TcpServer.h"

#include "OpcConstants.h"

namespace kp {
namespace opc {

typedef std::shared_ptr<class Server> ServerRef;

class Server : public std::enable_shared_from_this<Server> {
public:
	static ServerRef create(int numLEDs, int32_t port = kp::opc::DEFAULT_PORT, kp::opc::ColorOrder colorOrder = kp::opc::DEFAULT_COLOR_ORDER);

	~Server();

	// Support just a single channel for now
	const std::vector<ci::Color8u> getLeds() const;

	const unsigned long getNumMessagesReceived();

protected:
	Server(int numLEDs, int32_t port, kp::opc::ColorOrder colorOrder);

private:
	int mNumLeds;
	unsigned long mNumMessagesReceived;
	std::vector<std::vector<ci::Color8u>> mChannels;

	TcpServerRef mTcpServer;
	TcpSessionRef mTcpSession;
	int32_t mPort;
	kp::opc::ColorOrder mColorOrder;

	void accept();
	void onAccept(TcpSessionRef session);
	void onCancel();
	void onClose();
	void onError(std::string err, size_t bytesTransferred);
	void onRead(ci::BufferRef buffer);
	void onReadComplete();
	void parseBufferWithOffset(ci::BufferRef buffer, int offset);
};

} // namespace opc
} // namespace kp