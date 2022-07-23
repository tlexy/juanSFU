#include "signaling_handle.h"
#include <websocket/ws_connection.h>
#include <juansfu/utils/sutil.h>
#include <juansfu/udp/udp_receiver.h>
#include <juansfu/utils/global.h>
#include <juansfu/signaling/room_member.h>

void SignalingHandle::handle(const Json::Value& msg, std::shared_ptr<uvcore::TcpConnection> ptr)
{
	std::string cmd = GET_JSON_STRING(msg, "cmd", "");
	if (cmd == "join")
	{
		handle_join(msg, ptr);
	}
	else if (cmd == "publish")
	{
		handle_publish(msg, ptr); 
	}
	else
	{
		std::cerr << "undefined cmd: " << cmd << std::endl;
	}
}

void SignalingHandle::handle_join(const Json::Value& msg, std::shared_ptr<uvcore::TcpConnection> ptr)
{
	std::string sroomid = GET_JSON_STRING(msg, "roomId", "");
	std::string uid = GET_JSON_STRING(msg, "uid", "");
	if (sroomid.size() < 1 || uid.size() < 1)
	{
		std::cerr << "json format error." << __FUNCTION__  << ", line: " << __LINE__ << std::endl;
		return;
	}

	int64_t roomid = std::stoll(sroomid.c_str());
	auto roomptr = find_room(roomid);
	if (roomptr == nullptr)
	{
		roomptr = create_room(roomid);
	}
	
	auto member = std::make_shared<RoomMember>();
	member->uid = uid;
	member->offer_sdp = std::make_shared<SessionDescription>();

	Json::Value ret_json = Json::nullValue;
	ret_json["cmd"] = "resp-join";
	ret_json["roomId"] = sroomid;
	ret_json["uid"] = uid;
	ret_json["members"] = Json::arrayValue;
	for (auto it = roomptr->members.begin(); it != roomptr->members.end(); ++it)
	{
		Json::Value unit;
		unit["uid"] = it->second->uid;
		ret_json["members"].append(unit);
	}

	roomptr->connections[uid] = ptr;
	roomptr->members[uid] = member;

	Json::StreamWriterBuilder wbuilder;
	wbuilder["indentation"] = "";
	std::string send_msg = Json::writeString(wbuilder, ret_json);
	auto pptr = std::dynamic_pointer_cast<uvcore::WsConnection>(ptr);
	if (pptr)
	{
		pptr->write(send_msg.c_str(), send_msg.size(), OpCode::WsTextFrame);
	}
	_connections[ptr->id()] = roomid;
}

void SignalingHandle::handle_publish(const Json::Value& msg, std::shared_ptr<uvcore::TcpConnection> ptr)
{
	std::string sroomid = GET_JSON_STRING(msg, "roomId", "");
	std::string uid = GET_JSON_STRING(msg, "uid", "");
	if (sroomid.size() < 1 || uid.size() < 1)
	{
		std::cerr << "json format error." << __FUNCTION__ << ", line: " << __LINE__ << std::endl;
		return;
	}

	int64_t roomid = std::stoll(sroomid.c_str());
	auto roomptr = find_room(roomid);
	if (roomptr == nullptr)
	{
		std::cerr << "roomid not found in publish, roomid: " << roomid << ", uid: " << uid << std::endl;
		return;
	}
	auto it = roomptr->members.find(uid);
	if (it == roomptr->members.end())
	{
		std::cerr << "uid not found in publish, roomid: " << roomid << ", uid: " << uid << std::endl;
		return;
	}
	auto member = it->second;
	std::string sdp = GET_JSON_STRING(msg, "sdp", "");
	static std::string start = "sdp\":\"";
	auto pos = sdp.find(start);
	if (pos == std::string::npos)
	{
		std::cerr << "sdp format error. " << roomid << ", uid: " << uid << std::endl;
		return;
	}
	std::string sdpstr = sdp.substr(pos + start.size(), sdp.size() - pos - 2 - start.size() - 4);
	std::cout << "publish uid: " << uid << ", sdp: " << sdpstr << std::endl;

	std::vector<std::string> vecs;
	SUtil::split(sdpstr, "\\r\\n", vecs);
	if (vecs.size() < 6)
	{
		std::cerr << "sdp error, roomid: " << roomid << ", uid: " << uid  << ", sdp: " << sdpstr << std::endl;
		return;
	}

	bool flag = member->offer_sdp->parse_sdp(vecs);
	if (flag)
	{
		uvcore::IpAddress addr(7000);
		addr.setIp("192.168.101.40");//127.0.0.1
		RTCOfferAnswerOptions options;
		options.send_audio = false;
		options.send_video = false;
		options.recv_audio = true;
		options.recv_video = true;
		options.use_rtcp_mux = true;
		member->answer_sdp = std::make_shared<SessionDescription>();
		member->answer_sdp->create_answer(options, addr);
		member->answer_sdp->build(member->offer_sdp);
		std::string ans_offer = member->answer_sdp->to_string();

		Json::Value ret_json = Json::nullValue;
		ret_json["cmd"] = "resp-publish";
		ret_json["roomId"] = sroomid;
		ret_json["uid"] = uid;
		ret_json["sdp"] = Json::nullValue;
		ret_json["sdp"]["type"] = "answer";
		ret_json["sdp"]["sdp"] = ans_offer;

		auto pptr = std::dynamic_pointer_cast<uvcore::WsConnection>(ptr);
		if (pptr)
		{
			Json::StreamWriterBuilder wbuilder;
			wbuilder["indentation"] = "";
			std::string send_msg = Json::writeString(wbuilder, ret_json);

			pptr->write(send_msg.c_str(), send_msg.size(), OpCode::WsTextFrame);
		}
		member->start_recv(addr);
	}
	int a = 1;
}

void SignalingHandle::remove(std::shared_ptr<uvcore::TcpConnection> ptr)
{
	std::cout << "remove ptr, id=" << ptr->id() << std::endl;
	auto it = _connections.find(ptr->id());
	if (it != _connections.end())
	{
		auto rit = _all_rooms.find(it->second);
		if (rit != _all_rooms.end())
		{
			for (auto sit = rit->second->connections.begin(); sit != rit->second->connections.end();
				++sit)
			{
				if (sit->second->id() == ptr->id())
				{
					std::cerr << "remove uid: " << sit->first << std::endl;
					rit->second->members.erase(sit->first);
					rit->second->connections.erase(sit);
					break;
				}
			}
		}
		_connections.erase(it);
	}
}

std::shared_ptr<Room> SignalingHandle::find_room(int64_t roomid)
{
	auto it = _all_rooms.find(roomid);
	if (it != _all_rooms.end())
	{
		return it->second;
	}
	return nullptr;
}

std::shared_ptr<Room> SignalingHandle::create_room(int64_t roomid)
{
	auto ptr = std::make_shared<Room>();
	ptr->roomid = roomid;
	_all_rooms[roomid] = ptr;
	return ptr;
}