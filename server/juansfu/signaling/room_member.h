#ifndef JUAN_ROOM_MEMBER_H
#define JUAN_ROOM_MEMBER_H

#include <string>
#include <memory>
#include <unordered_map>
#include <juansfu/sdp/session_description.h>
#include <uvnet/core/udp_server.h>
#include <uvnet/core/ip_address.h>
//#include <juansfu/rtc_base/ip_address.h>

class UdpReceiver;
class IceConnection;
class RtcDtls;

class RoomMember
{
public:
	void start_recv(const uvcore::IpAddress&);
	void stop_recv();
public:
	std::string uid;
	std::shared_ptr<SessionDescription> offer_sdp;
	std::shared_ptr<SessionDescription> answer_sdp;
	std::shared_ptr<UdpReceiver> udp_receiver;
	//key为ip:port pair
	std::unordered_map<std::string, std::shared_ptr<IceConnection>> ice_connections;
	std::unordered_map<std::string, std::shared_ptr<RtcDtls>> dtls_connections;

private:
	uvcore::IpAddress _addr;

private:
	void on_udp_receive(uvcore::Udp*, const uvcore::IpAddress&);
};

#endif