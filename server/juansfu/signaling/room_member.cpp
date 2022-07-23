#include <juansfu/signaling/room_member.h>
#include <juansfu/udp/udp_receiver.h>
#include <juansfu/utils/global.h>
#include <juansfu/udp/stun.h>

void RoomMember::start_recv(const uvcore::IpAddress& addr)
{
	using namespace std::placeholders;

	_addr = addr;
	udp_receiver = std::make_shared<UdpReceiver>(addr, Global::GetInstance()->get_udp_server());
	udp_receiver->set_data_cb(std::bind(&RoomMember::on_udp_receive, this, _1, _2));
	udp_receiver->start();
}

void RoomMember::on_udp_receive(uvcore::Udp* udp, const struct sockaddr* addr)
{
	StunPacket* sp = StunPacket::parse(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size());
	udp->get_inner_buffer()->reset();

	bool flag = true;
	if (answer_sdp->media_contents.size() > 0)
	{
		flag = sp->validate(answer_sdp->media_contents[0]->ice, 
			udp->get_inner_buffer()->read_ptr(), 
			udp->get_inner_buffer()->readable_size());
	}
}