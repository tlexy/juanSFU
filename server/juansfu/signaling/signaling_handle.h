#ifndef JUAN_SIGNALING_HANDLE_H
#define JUAN_SIGNALING_HANDLE_H

#include <juansfu/utils/singleton.h>
#include <juansfu/utils/sutil.h>
#include <string>
#include <stdint.h>
#include <unordered_map>
#include <juansfu/sdp/session_description.h>

namespace uvcore
{
	class TcpConnection;
}

class UdpReceiver;

class RoomMember
{
public:
	std::string uid;
	std::shared_ptr<SessionDescription> offer_sdp;
	std::shared_ptr<SessionDescription> answer_sdp;
	std::shared_ptr<UdpReceiver> udp_receiver;
};

class Room
{
public:
	int64_t roomid;
	std::unordered_map<std::string, std::shared_ptr<RoomMember>> members;
	std::unordered_map<std::string, std::shared_ptr<uvcore::TcpConnection>> connections;
};

class SignalingHandle : public Singleton<SignalingHandle>
{
public:
	void handle(const Json::Value& msg, std::shared_ptr<uvcore::TcpConnection>);

	void remove(std::shared_ptr<uvcore::TcpConnection>);

private:
	void handle_join(const Json::Value& msg, std::shared_ptr<uvcore::TcpConnection>);
	void handle_publish(const Json::Value& msg, std::shared_ptr<uvcore::TcpConnection>);

	std::shared_ptr<Room> find_room(int64_t roomid);
	std::shared_ptr<Room> create_room(int64_t roomid);

private:
	std::unordered_map<int64_t, std::shared_ptr<Room>> _all_rooms;
	std::unordered_map<int64_t, int64_t> _connections;
};

#endif