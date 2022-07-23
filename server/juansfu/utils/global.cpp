#include "global.h"
#include <juansfu/utils/sutil.h>
#include <juansfu/rtc_base/rtc_certificate_generator.h>
#include <juansfu/rtc_base/logging.h>

#include <core/udp_server.h>
#include <server/thread_timer_eventloop.h>

const uint64_t k_year_in_ms = 365 * 24 * 3600 * 1000L;

void Global::init()
{
    if (_generate_and_check_certificate() != 0)
    {
        std::cerr << "_generate_and_check_certificate failed." << std::endl;
    }

    _loop = std::make_shared<uvcore::ThreadTimerEventLoop>();
    _loop->init(3000, nullptr);
    _loop->start_thread();

    _udp_server = std::make_shared<uvcore::UdpServer>(_loop->get_loop());
}

rtc::scoped_refptr<rtc::RTCCertificate> Global::get_dtls_certificate()
{
    return _certificate;
}

std::shared_ptr<uvcore::UdpServer> Global::get_udp_server()
{
    return _udp_server;
}

int Global::_generate_and_check_certificate() 
{
    if (!_certificate || _certificate->HasExpired(SUtil::getTimeStampMilli())) 
    {
        rtc::KeyParams key_params;
        _certificate = rtc::RTCCertificateGenerator::GenerateCertificate(key_params,
            k_year_in_ms);
        if (_certificate) 
        {
            rtc::RTCCertificatePEM pem = _certificate->ToPEM();
            RTC_LOG(LS_INFO) << "rtc certificate: \n" << pem.certificate();
        }
    }

    if (!_certificate) 
    {
        RTC_LOG(LS_WARNING) << "get certificate error";
        return -1;
    }

    return 0;
}