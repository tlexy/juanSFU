#ifndef JUAN_RTC_STREAM_MGR_H
#define JUAN_RTC_STREAM_MGR_H

#include <unordered_map>
#include <string>
#include <memory>
#include <juansfu/utils/singleton.h>
#include <utility>

class SrtpSubscriber;
class SrtpTransport;
class RtcDtls;

class RtcStreamMgr : public Singleton<RtcStreamMgr>
{
public:
	void add_stream(const std::string& uid, const std::string& addr, std::shared_ptr<SrtpTransport>);
	//uid订阅target_uid
	void add_subscriber(const std::string& uid, const std::string& target_uid);

private:
	std::unordered_map<std::string, std::pair<std::string, std::string>> _subs_map;
	std::unordered_map<std::string, std::shared_ptr<SrtpTransport>> _streams;
};

#endif