#include <juansfu/udp/udp_receiver.h>
#include <iostream>
#include <juansfu/udp/stun.h>

UdpReceiver::UdpReceiver(const uvcore::IpAddress& addr,
	std::shared_ptr<uvcore::UdpServer> server)
	:_addr(addr),
	_udp_server(server)
{}

void UdpReceiver::start()
{
	using namespace std::placeholders;
	if (!_udp_server)
	{
		std::cerr << "bind udp error." << std::endl;
		return;
	}
	std::cout << "bind udp success." << std::endl;
	_udp = _udp_server->addBind(_addr, std::bind(&UdpReceiver::on_udp_receive, this, _1, _2));
}

void UdpReceiver::on_udp_receive(uvcore::Udp* udp, const struct sockaddr* addr)
{
	std::cout << "recv udp data, len: " << udp->get_inner_buffer()->readable_size() << std::endl;

	StunPacket* sp = StunPacket::parse(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size());
	udp->get_inner_buffer()->reset();
}