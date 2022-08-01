#ifndef JUAN_SRTP_SESSION_H
#define JUAN_SRTP_SESSION_H

#include <srtp2/srtp.h>
#include <stdint.h>
#include <vector>

class SrtpSession
{
public:
	SrtpSession();

	bool set_send(int cs, const uint8_t* key, size_t key_len,
		const std::vector<int>& extension_ids);
	bool update_send(int cs, const uint8_t* key, size_t key_len,
		const std::vector<int>& extension_ids);
	bool set_recv(int cs, const uint8_t* key, size_t key_len,
		const std::vector<int>& extension_ids);
	bool update_recv(int cs, const uint8_t* key, size_t key_len,
		const std::vector<int>& extension_ids);

	void handle_event(srtp_event_data_t* ev);

	bool unprotect_rtp(void* p, int in_len, int* out_len);
	bool unprotect_rtcp(void* p, int in_len, int* out_len);
	bool protect_rtp(void* p, int in_len, int max_len, int* out_len);
	bool protect_rtcp(void* p, int in_len, int max_len, int* out_len);

	void get_auth_tag_len(int* rtp_auth_tag_len, int* rtcp_auth_tag_len);

private:
	bool set_key(int type, int cs, const uint8_t* key, size_t key_len,
		const std::vector<int>& extension_ids);

private:
	srtp_ctx_t_* _srtp_session = nullptr;
	int _rtp_auth_tag_len = 0;
	int _rtcp_auth_tag_len = 0;
};

#endif