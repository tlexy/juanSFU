#include <juansfu/udp/rtc_dtls.h>
#include <juansfu/utils/global.h>

#include <iostream>

bool RtcDtls::is_dtls(const uint8_t* data, size_t len)
{
	//https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
	return ((len >= 13) && (data[0] > 19 && data[0] < 64));
}

bool RtcDtls::is_dtls_client_hello_packet(const uint8_t* data, size_t len)
{
	if (!is_dtls(data, len)) {
		return false;
	}

	//const uint8_t* u = reinterpret_cast<const uint8_t*>(buf);
	return len > 17 && (data[0] == 22 && data[13] == 1);
}

void RtcDtls::set_remote_dtls(const std::string& alg, const std::string& fp)
{
	if (_remote_fp == fp && !alg.empty())
	{
		std::cerr << "dtls crypto not change." << std::endl;
	}

	_remote_alg = alg;
	_remote_fp = fp;
}

void RtcDtls::handle_dtls(const uint8_t* data, size_t len)
{}