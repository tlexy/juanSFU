﻿#include <juansfu/udp/udp_receiver.h>
#include <iostream>
#include <juansfu/udp/stun.h>

UdpReceiver::UdpReceiver(const uvcore::IpAddress& addr,
	std::shared_ptr<uvcore::UdpServer> server)
	:_addr(addr),
	_udp_server(server)
{}

void UdpReceiver::set_data_cb(ReceiverDataCb cb)
{
	_data_cb = cb;
}

void UdpReceiver::start()
{
	using namespace std::placeholders;
	if (!_udp_server)
	{
		std::cerr << "bind udp error." << std::endl;
		return;
	}
	std::cout << "start to bind udp addr: " << _addr.toString() << std::endl;
	_udp = _udp_server->addBind(_addr, std::bind(&UdpReceiver::on_udp_receive, this, _1, _2));
}

void UdpReceiver::on_udp_receive(uvcore::Udp* udp, const struct sockaddr* addr)
{
	//std::cout << "recv udp data, len: " << udp->get_inner_buffer()->readable_size() << std::endl;
	if (!udp || !addr)
	{
		std::cerr << "UDP RECEIVE ERROR." << __FUNCTION__ << std::endl;
		return;
	}
	if (_data_cb)
	{
		uvcore::IpAddress ipaddr = uvcore::IpAddress::fromRawSocketAddress((sockaddr*)addr, sizeof(struct sockaddr_in));
		_data_cb(udp, ipaddr);
	}
}

void UdpReceiver::stop()
{
	_udp->close(std::bind(&UdpReceiver::deleter, _udp, std::placeholders::_1));
}

void UdpReceiver::deleter(uvcore::Udp* udp, int64_t)
{
	delete udp;
}