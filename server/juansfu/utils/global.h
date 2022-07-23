#ifndef JUAN_GLOBAL_H
#define JUAN_GLOBAL_H

#include <juansfu/utils/singleton.h>
#include <juansfu/rtc_base/rtc_certificate.h>

namespace uvcore {
	class ThreadTimerEventLoop;
	class UdpServer;
}

class Global : public Singleton<Global>
{
public:
	void init();

	rtc::scoped_refptr<rtc::RTCCertificate>
		get_dtls_certificate();

	std::shared_ptr<uvcore::UdpServer> get_udp_server();

private:
	int _generate_and_check_certificate();

private:
	rtc::scoped_refptr<rtc::RTCCertificate> _certificate;

	std::shared_ptr<uvcore::ThreadTimerEventLoop> _loop;
	std::shared_ptr<uvcore::UdpServer> _udp_server;
};

#endif