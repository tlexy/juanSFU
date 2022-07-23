#ifndef JUAN_UDP_RECEIVER_H
#define JUAN_UDP_RECEIVER_H

#include <memory>
#include <functional>
#include <uvnet/core/ip_address.h>
#include <uvnet/core/udp_server.h>

class UdpReceiver
{
public:
	UdpReceiver(const uvcore::IpAddress& addr,
		std::shared_ptr<uvcore::UdpServer> server);

	void start();

private:
	void on_udp_receive(uvcore::Udp*, const struct sockaddr*);

private:
	uvcore::IpAddress _addr;
	std::shared_ptr<uvcore::UdpServer> _udp_server{ nullptr };
	uvcore::Udp* _udp{ nullptr };
	bool _is_start{ false };
};

#endif