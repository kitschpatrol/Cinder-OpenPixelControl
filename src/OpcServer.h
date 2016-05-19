//
//  OPCServer.hpp
//  CinderOpenPixelControlSandbox
//
//  Created by Eric Mika on 12/23/15.
//
//

#pragma once

#include "TcpServer.h"
#include "cinder/Color.h"
#include "OpcConstants.h"
#include <deque>

namespace kp {
namespace opc {

typedef std::shared_ptr<class Server> ServerRef;

class Server : public std::enable_shared_from_this<Server> {
public:
	static ServerRef create(int numLEDs, int32_t port = kp::opc::DEFAULT_PORT, kp::opc::ColorOrder colorOrder = kp::opc::DEFAULT_COLOR_ORDER);

	~Server();

	// Support just a single channel for now
	const std::vector<ci::Color> getLeds() const;

	const unsigned long getNumMessagesReceived();

protected:
	Server(int numLEDs, int32_t port, kp::opc::ColorOrder colorOrder);

private:
	int mNumLeds;
	unsigned long mNumMessagesReceived;
	std::vector<std::vector<ci::Color>> mChannels;

	TcpServerRef mTcpServer;
	TcpSessionRef mTcpSession;
	int32_t mPort;
	kp::opc::ColorOrder mColorOrder;
	std::deque<uint8_t> mBufferQueue;

	void accept();
	void onAccept(TcpSessionRef session);
	void onCancel();
	void onClose();
	void onError(std::string err, size_t bytesTransferred);
	void onRead(ci::BufferRef buffer);
	void onReadComplete();
	bool parseQueue(std::deque<uint8_t> &queue);
};

} // namespace opc
} // namespace kp