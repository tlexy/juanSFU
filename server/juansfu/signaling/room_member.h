#ifndef JUAN_ROOM_MEMBER_H
#define JUAN_ROOM_MEMBER_H

#include <string>
#include <memory>
#include <juansfu/sdp/session_description.h>
#include <uvnet/core/udp_server.h>
#include <uvnet/core/ip_address.h>

class UdpReceiver;

class RoomMember
{
public:
	void start_recv(const uvcore::IpAddress&);
public:
	std::string uid;
	std::shared_ptr<SessionDescription> offer_sdp;
	std::shared_ptr<SessionDescription> answer_sdp;
	std::shared_ptr<UdpReceiver> udp_receiver;

private:
	uvcore::IpAddress _addr;

private:
	void on_udp_receive(uvcore::Udp*, const struct sockaddr*);
};

#endif