#include <juansfu/signaling/room_member.h>
#include <juansfu/udp/udp_receiver.h>
#include <juansfu/utils/global.h>
#include <juansfu/udp/stun.h>
#include <juansfu/udp/ice_connection.h>
#include <juansfu/signaling/port_mgr.h>
#include <juansfu/udp/rtc_dtls.h>
#include <juansfu/rtprtcp/rtprtcp_pub.hpp>
#include <iostream>

void RoomMember::start_recv(const uvcore::IpAddress& addr)
{
	using namespace std::placeholders;

	_addr = addr;
	_addr.parse();
	udp_receiver = std::make_shared<UdpReceiver>(addr, Global::GetInstance()->get_udp_server());
	udp_receiver->set_data_cb(std::bind(&RoomMember::on_udp_receive, this, _1, _2));
	udp_receiver->start();
}

void RoomMember::stop_recv()
{
	udp_receiver->stop();
	int port = _addr.getPort();
	UdpPortManager::GetInstance()->recycle_port(port);
}

void RoomMember::on_udp_receive(uvcore::Udp* udp, const uvcore::IpAddress& addr)
{
	if (StunPacket::is_stun(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size()))
	{
		auto it = ice_connections.find(addr.toString());
		if (it != ice_connections.end())
		{
			it->second->on_udp_data(udp);
			return;
		}
		//new connection...
		StunPacket* sp = StunPacket::parse(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size());
		udp->get_inner_buffer()->reset();

		bool flag = true;
		if (answer_sdp->media_contents.size() > 0)
		{
			flag = sp->validate(answer_sdp->media_contents[0]->ice,
				udp->get_inner_buffer()->read_ptr(),
				udp->get_inner_buffer()->readable_size());
		}
		if (flag)
		{
			auto ptr = std::make_shared<IceConnection>(addr, _addr);
			ptr->set_ice_pwd(answer_sdp->media_contents[0]->ice.passwd);
			ice_connections[addr.toString()] = ptr;
			ptr->send_binding_response(udp, sp);
		}
		delete sp;
	}
	else if (is_rtcp(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size()))
	{
		//on_handle_rtcp_data(udp_data, udp_data_len, address);
		std::cerr << "is_rtcp protocol." << std::endl;
	}
	else if (is_rtp(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size())) {

		//on_handle_rtp_data(udp_data, udp_data_len, address);
		std::cerr << "is_rtp protocol." << std::endl;
	}
	else if (RtcDtls::is_dtls(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size()))
	{
	}
	else
	{
		std::cerr << "unknown protocol." << std::endl;
	}
	udp->get_inner_buffer()->reset();
}