#include <juansfu/udp/rtc_dtls.h>

bool RtcDtls::is_dtls(const uint8_t* data, size_t len)
{
	//https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
	return ((len >= 13) && (data[0] > 19 && data[0] < 64));
}