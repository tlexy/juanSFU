#ifndef JUAN_RTC_DTLS_H
#define JUAN_RTC_DTLS_H

#include <stdint.h>

class RtcDtls
{
public:
	static bool is_dtls(const uint8_t* data, size_t len);
};

#endif