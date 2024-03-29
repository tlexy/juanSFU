﻿#ifndef JUAN_ROOM_MEMBER_H
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
class SrtpTransport;

class MemberUdpPorts
{
public:
	std::shared_ptr<IceConnection> ice_connection = nullptr;
	std::shared_ptr<RtcDtls> dtls_connection = nullptr;
	std::shared_ptr<SrtpTransport> srtp = nullptr;
};

class RoomMember
{
public:
	void start_recv(const uvcore::IpAddress&);
	void stop_recv();
	void destory();
public:
	std::string uid;
	std::shared_ptr<SessionDescription> offer_sdp = nullptr;
	std::shared_ptr<SessionDescription> answer_sdp = nullptr;
	std::shared_ptr<UdpReceiver> udp_receiver = nullptr;
	//key为ip:port pair
	//std::unordered_map<std::string, std::shared_ptr<IceConnection>> ice_connections;
	//std::unordered_map<std::string, std::shared_ptr<RtcDtls>> dtls_connections;
	std::unordered_map<std::string, std::shared_ptr<MemberUdpPorts>> udp_handles;

private:
	uvcore::IpAddress _addr;

private:
	void on_udp_receive(uvcore::Udp*, const uvcore::IpAddress&);
};

#endif