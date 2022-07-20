#include "global.h"
#include <juansfu/utils/sutil.h>
#include <juansfu/rtc_base/rtc_certificate_generator.h>
#include <juansfu/rtc_base/logging.h>

const uint64_t k_year_in_ms = 365 * 24 * 3600 * 1000L;

void Global::init()
{
    if (_generate_and_check_certificate() != 0)
    {
        std::cerr << "_generate_and_check_certificate failed." << std::endl;
    }
}

rtc::scoped_refptr<rtc::RTCCertificate> Global::get_dtls_certificate()
{
    return _certificate;
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