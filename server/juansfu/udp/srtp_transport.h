#ifndef JUAN_SRTP_TRANSPORT_H
#define JUAN_SRTP_TRANSPORT_H

#include <uvnet/core/udp_server.h>
#include <uvnet/core/ip_address.h>
#include <stdint.h>
#include <unordered_map>

#include <srtp2/srtp.h>

class SrtpSubscriber;
class RtcDtls;
class SrtpSession;

class SrtpTransport
{
public:
	void handle_rtp_data(uvcore::Udp*);
	void handle_rtcp_data(uvcore::Udp*);
    //设置加解密参数
	bool set_srtp_param(int send_cs,
        const uint8_t* send_key,
        size_t send_key_len,
        const std::vector<int>& send_extension_ids,
        int recv_cs,
        const uint8_t* recv_key,
        size_t recv_key_len,
        const std::vector<int>& recv_extension_ids);

	void add_subscriber(const std::string& addr, std::shared_ptr<SrtpSubscriber>);
	void remove_subscriber(const std::string& addr);

private:
    static void event_handle_thunk(srtp_event_data_t* ev);

private:
	std::unordered_map<std::string, std::shared_ptr<SrtpSubscriber>> _subs_map;
    bool _srtp_init = false;

};

#endif