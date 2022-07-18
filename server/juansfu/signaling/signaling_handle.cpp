#include "signaling_handle.h"
#include <websocket/ws_connection.h>
#include <juansfu/utils/sutil.h>

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

void SignalingHandle::handle_publish(const Json::Value& msg, std::shared_ptr<uvcore::TcpConnection>)
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