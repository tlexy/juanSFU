#include <juansfu/signaling/room_member.h>
#include <juansfu/udp/udp_receiver.h>
#include <juansfu/utils/global.h>
#include <juansfu/udp/stun.h>
#include <juansfu/udp/ice_connection.h>

void RoomMember::start_recv(const uvcore::IpAddress& addr)
{
	using namespace std::placeholders;

	_addr = addr;
	udp_receiver = std::make_shared<UdpReceiver>(addr, Global::GetInstance()->get_udp_server());
	udp_receiver->set_data_cb(std::bind(&RoomMember::on_udp_receive, this, _1, _2));
	udp_receiver->start();
}

void RoomMember::stop_recv()
{
	udp_receiver->stop();
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
			ice_connections[addr.toString()] = ptr;
			ptr->send_binding_response(udp, sp, answer_sdp->media_contents[0]->ice.passwd);
		}
		delete sp;
	}
}