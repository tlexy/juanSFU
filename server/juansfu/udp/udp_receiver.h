#ifndef JUAN_UDP_RECEIVER_H
#define JUAN_UDP_RECEIVER_H

#include <memory>
#include <functional>
#include <uvnet/core/ip_address.h>
#include <uvnet/core/udp_server.h>

using ReceiverDataCb = std::function<void(uvcore::Udp*, const uvcore::IpAddress&)>;

class UdpReceiver
{
public:
	UdpReceiver(const uvcore::IpAddress& addr,
		std::shared_ptr<uvcore::UdpServer> server);

	void set_data_cb(ReceiverDataCb cb);
	void start();
	void stop();

private:
	void on_udp_receive(uvcore::Udp*, const struct sockaddr*);
	static void deleter(uvcore::Udp*, int64_t);

private:
	uvcore::IpAddress _addr;
	ReceiverDataCb _data_cb{ nullptr };
	std::shared_ptr<uvcore::UdpServer> _udp_server{ nullptr };
	uvcore::Udp* _udp{ nullptr };
	bool _is_start{ false };
};

#endif