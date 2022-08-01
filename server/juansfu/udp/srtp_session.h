#ifndef JUAN_SRTP_SESSION_H
#define JUAN_SRTP_SESSION_H

#include <srtp2/srtp.h>
#include <stdint.h>
#include <vector>

class SrtpSession
{
public:
	SrtpSession();

private:
	void set_key(int type, int cs, const uint8_t* key, size_t key_len,
		const std::vector<int>& extension_ids);

private:
	srtp_ctx_t_* _srtp_session = nullptr;
	int _rtp_auth_tag_len = 0;
	int _rtcp_auth_tag_len = 0;
};

#endif