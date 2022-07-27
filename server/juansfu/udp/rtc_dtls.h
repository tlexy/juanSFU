#ifndef JUAN_RTC_DTLS_H
#define JUAN_RTC_DTLS_H

#include <stdint.h>
#include <string>
#include <juansfu/rtc_base/ssl_stream_adapter.h>
#include <juansfu/rtc_base/buffer_queue.h>
#include <vector>
#include <uvnet/core/udp_server.h>
#include <uvnet/core/ip_address.h>

class DtlsStreamInterface : public rtc::StreamInterface
{
public:
    DtlsStreamInterface(uvcore::Udp*, const uvcore::IpAddress& remote_addr);
    rtc::StreamState GetState() const override;
    rtc::StreamResult Read(void* buffer,
        size_t buffer_len,
        size_t* read,
        int* error) override;
    rtc::StreamResult Write(const void* data,
        size_t data_len,
        size_t* written,
        int* error) override;
    void Close() override;

    bool on_packet_receive(const uint8_t* data, size_t len);

private:
    rtc::BufferQueue _que;
    rtc::StreamState _state = rtc::SS_OPEN;
    uvcore::Udp* _udp;
    uvcore::IpAddress _remote_addr;

};

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
    RtcDtls(uvcore::Udp*, const uvcore::IpAddress& remote_addr);
	static bool is_dtls(const uint8_t* data, size_t len);
    static bool is_dtls_client_hello_packet(const uint8_t* data, size_t len);

    void set_remote_dtls(const std::string& alg, const std::string& fp);
    bool setup_dtls();
    void handle_dtls(const uint8_t* data, size_t len);

private:
    std::string _remote_alg;
    std::string _remote_fp;
    DtlsTransportState _state = DtlsTransportState::k_new;
    std::unique_ptr<rtc::SSLStreamAdapter> _dtls_adapter;
    DtlsStreamInterface* _downward = nullptr;
    std::vector<int> _srtp_ciphers;
};

#endif