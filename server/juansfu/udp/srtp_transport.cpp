#include <juansfu/udp/srtp_subscriber.h>
#include <juansfu/udp/srtp_transport.h>
#include <iostream>
#include <juansfu/udp/srtp_session.h>

bool SrtpTransport::_srtp_init = false;

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
    if (!_send_session)
    {
        _send_session = std::make_shared<SrtpSession>();
    }
    if (!_recv_session)
    {
        _recv_session = std::make_shared<SrtpSession>();
    }
    if (_send_session)
    {
        _send_session->set_send(send_cs, send_key, send_key_len, send_extension_ids);
    }
    else 
    {
        return false;
    }
    if (_recv_session)
    {
        _recv_session->set_recv(recv_cs, recv_key, recv_key_len, recv_extension_ids);
    }
    else
    {
        return false;
    }
    return true;
}

void SrtpTransport::reset()
{
    _send_session = nullptr;
    _recv_session = nullptr;
    std::cerr << "srtp transport is reset" << std::endl;
}

void SrtpTransport::event_handle_thunk(srtp_event_data_t* ev)
{
    SrtpSession* session = (SrtpSession*)(srtp_get_user_data(ev->session));
    if (session) {
        session->handle_event(ev);
    }
}

void SrtpTransport::get_send_auth_tag_len(int* rtp_auth_tag_len, int* rtcp_auth_tag_len)
{
    if (_send_session) 
    {
        _send_session->get_auth_tag_len(rtp_auth_tag_len, rtcp_auth_tag_len);
    }
}