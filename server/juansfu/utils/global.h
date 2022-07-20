#ifndef JUAN_GLOBAL_H
#define JUAN_GLOBAL_H

#include <juansfu/utils/singleton.h>
#include <juansfu/rtc_base/rtc_certificate.h>

class Global : public Singleton<Global>
{
public:
	void init();

	rtc::scoped_refptr<rtc::RTCCertificate>
		get_dtls_certificate();

private:
	int _generate_and_check_certificate();

private:
	rtc::scoped_refptr<rtc::RTCCertificate> _certificate;
};

#endif