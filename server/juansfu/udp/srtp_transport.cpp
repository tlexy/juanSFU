#include <juansfu/udp/srtp_transport.h>
#include <iostream>
#include <juansfu/udp/srtp_session.h>
#include <juansfu/rtc_base/copy_on_write_buffer.h>

bool SrtpTransport::_srtp_init = false;

SrtpTransport::SrtpTransport(uvcore::Udp* udp, const uvcore::IpAddress& addr)
    :_udp(udp),
    _addr(addr)
{}

void SrtpTransport::handle_rtp_data(uvcore::Udp* udp)
{
    if (!_srtp_init)
    {
        std::cerr << "unprotect rtp failed, _srtp_init is nullptr" << std::endl;
        return;
    }
    //1. 解密
    //2. 转发给订阅者
    std::string data((const char*)udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size());
    int out_len = 0;
    bool flag = _recv_session->unprotect_rtp((void*)data.c_str(), data.size(), &out_len);
    if (out_len > 0 && flag)
    {
        for (auto it = _subs_map.begin(); it != _subs_map.end(); ++it)
        {
            it->second->on_rtp_packet((void*)data.c_str(), out_len);
        }
    }
    else
    {
        std::cerr << "unprotect rtp failed, out_len = " << out_len << std::endl;
    }
}

void SrtpTransport::handle_rtcp_data(uvcore::Udp* udp)
{
    if (!_srtp_init)
    {
        std::cerr << "unprotect rtp failed, _srtp_init is nullptr" << std::endl;
        return;
    }
    //1. 解密
    //2. 转发给订阅者
    std::string data((const char*)udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size());
    int out_len = 0;
    bool flag = _recv_session->unprotect_rtcp((void*)data.c_str(), data.size(), &out_len);
    if (out_len > 0 && flag)
    {
        for (auto it = _subs_map.begin(); it != _subs_map.end(); ++it)
        {
            it->second->on_rtcp_packet((void*)data.c_str(), out_len);
        }
    }
    else
    {
        std::cerr << "unprotect rtp failed, out_len = " << out_len << std::endl;
    }
}

void SrtpTransport::add_subscriber(const std::string& uid, std::shared_ptr<SrtpSubscriber> sber)
{
	_subs_map[uid] = sber;
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

void SrtpTransport::on_rtp_packet(void* data, int len)
{
    if (!_srtp_init)
    {
        std::cerr << "on_rtp_packet rtp failed, _srtp_init is nullptr" << std::endl;
        return;
    }
    //std::cout << "receive peer rtp data, len = " << len << std::endl;
    int rtp_auth_tag_len = 0;
    get_send_auth_tag_len(&rtp_auth_tag_len, nullptr);

    rtc::CopyOnWriteBuffer packet((char*)data, len, len + rtp_auth_tag_len);

    char* buf = (char*)packet.data();
    int size = packet.size();
    bool flag = _send_session->protect_rtp(buf, size, packet.capacity(), &size);
    if (flag)
    {
        _udp->send2(buf, size, _addr);
    }
    else
    {
        std::cerr << "protect rtp failed." << std::endl;
    }
}

void SrtpTransport::on_rtcp_packet(void* data, int len)
{
    if (!_srtp_init)
    {
        std::cerr << "on_rtcp_packet rtp failed, _srtp_init is nullptr" << std::endl;
        return;
    }
    //std::cout << "receive peer rtcp data, len = " << len << std::endl;
    int rtcp_auth_tag_len = 0;
    get_send_auth_tag_len(nullptr, &rtcp_auth_tag_len);
    rtc::CopyOnWriteBuffer packet((char*)data, len, len + rtcp_auth_tag_len + sizeof(uint32_t));

    char* buf = (char*)packet.data();
    int size = packet.size();
    bool flag = _send_session->protect_rtcp(buf, size, packet.capacity(), &size);
    if (flag)
    {
        _udp->send2(buf, size, _addr);
    }
    else
    {
        std::cerr << "protect rtcp failed." << std::endl;
    }
}