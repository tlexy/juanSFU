#include <juansfu/udp/srtp_subscriber.h>
#include <juansfu/udp/srtp_transport.h>
#include <iostream>

void SrtpTransport::handle_rtp_data(uvcore::Udp*)
{}

void SrtpTransport::handle_rtcp_data(uvcore::Udp*)
{}

void SrtpTransport::add_subscriber(const std::string& addr, std::shared_ptr<SrtpSubscriber> sber)
{
	_subs_map[addr] = sber;
}

void SrtpTransport::remove_subscriber(const std::string& addr)
{
	_subs_map.erase(addr);
}

bool SrtpTransport::set_srtp_param(int send_cs,
    const uint8_t* send_key,
    size_t send_key_len,
    const std::vector<int>& send_extension_ids,
    int recv_cs,
    const uint8_t* recv_key,
    size_t recv_key_len,
    const std::vector<int>& recv_extension_ids)
{
    if (!_srtp_init)
    {
        int err = srtp_init();
        if (err != srtp_err_status_ok) 
        {
            std::cerr << "Failed to init srtp, err: " << err << std::endl;
            return false;
        }

        _srtp_init = true;

        err = srtp_install_event_handler(&SrtpTransport::event_handle_thunk);
        if (err != srtp_err_status_ok) 
        {
            std::cerr << "Failed to install srtp event, err: " << err << std::endl;
            return false;
        }
    }
}