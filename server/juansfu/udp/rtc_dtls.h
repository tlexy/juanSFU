#ifndef JUAN_RTC_DTLS_H
#define JUAN_RTC_DTLS_H

#include <stdint.h>
#include <string>

class RtcDtls
{
public:
    enum class DtlsTransportState {
        k_new,
        k_connecting,
        k_connected,
        k_closed,
        k_failed,
        k_num_values
    };
	static bool is_dtls(const uint8_t* data, size_t len);
    static bool is_dtls_client_hello_packet(const uint8_t* data, size_t len);

    void set_remote_dtls(const std::string& alg, const std::string& fp);

    void handle_dtls(const uint8_t* data, size_t len);

private:
    std::string _remote_alg;
    std::string _remote_fp;
    DtlsTransportState _state = DtlsTransportState::k_new;
};

#endif