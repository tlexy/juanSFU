#include <juansfu/udp/rtc_stream_mgr.h>
#include <juansfu/udp/srtp_transport.h>

void RtcStreamMgr::add_subscriber(const std::string& uid, const std::string& target_uid)
{
	_subs_map[uid] = std::make_pair(target_uid, "");
}

/// <summary>
/// 一个新的流建立后，应该将订阅列表中的用户加入到订阅中
/// </summary>
/// <param name="uid"></param>
/// <param name="addr"></param>
/// <param name=""></param>
void RtcStreamMgr::add_stream(const std::string& uid, const std::string& addr, std::shared_ptr<SrtpTransport> st)
{
	_streams[uid] = st;
	//查找该用户的订阅
	auto it = _subs_map.find(uid);
	if (it != _subs_map.end())
	{
		//如果订阅的流已经存在，那就相互订阅
		auto sit = _streams.find(it->second.first);
		if (sit != _streams.end())
		{
			//sit->second->add_subscriber(it->second.first, st);
			//st->add_subscriber(uid, sit->second);
			sit->second->add_subscriber(uid, st);
			st->add_subscriber(it->second.first, sit->second);
		}
	}
}

void RtcStreamMgr::remove_subscriber(const std::string& uid)
{
	auto it = _subs_map.find(uid);
	if (it == _subs_map.end())
	{
		return;
	}
	auto sit = _streams.find(uid);
	if (sit != _streams.end())
	{
		sit->second->remove_subscriber(it->second.first);
	}
	sit = _streams.find(it->second.first);
	if (sit != _streams.end())
	{
		sit->second->remove_subscriber(uid);
	}

}