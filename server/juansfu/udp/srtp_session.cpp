#include <juansfu/udp/srtp_session.h>
#include <iostream>

SrtpSession::SrtpSession()
{}

bool SrtpSession::set_send(int cs, const uint8_t* key, size_t key_len,
    const std::vector<int>& extension_ids)
{
    return set_key(ssrc_any_outbound, cs, key, key_len, extension_ids);
}

bool SrtpSession::update_send(int cs, const uint8_t* key, size_t key_len,
    const std::vector<int>& extension_ids)
{
    return false;
}

bool SrtpSession::set_recv(int cs, const uint8_t* key, size_t key_len,
    const std::vector<int>& extension_ids)
{
    return set_key(ssrc_any_inbound, cs, key, key_len, extension_ids);
}

bool SrtpSession::update_recv(int cs, const uint8_t* key, size_t key_len,
    const std::vector<int>& extension_ids)
{
    return false;
}

bool SrtpSession::set_key(int type, int cs, const uint8_t* key, size_t key_len,
    const std::vector<int>& extension_ids)
{
    srtp_policy_t policy;
    memset(&policy, 0, sizeof(policy));

    bool rtp_ret = srtp_crypto_policy_set_from_profile_for_rtp(
        &policy.rtp, (srtp_profile_t)cs);
    bool rtcp_ret = srtp_crypto_policy_set_from_profile_for_rtcp(
        &policy.rtcp, (srtp_profile_t)cs);
    if (rtp_ret != srtp_err_status_ok || rtcp_ret != srtp_err_status_ok)
    {
        std::cerr << "SRTP session " << (_srtp_session ? "create" : "update")
            << " failed: unsupported crypto suite " << cs << std::endl;
        return false;
    }
    if (!key || key_len != (size_t)policy.rtp.cipher_key_len)
    {
        std::cerr << "SRTP session " << (_srtp_session ? "create" : "update")
            << " failed: invalid key" << std::endl;
        return false;
    }

    policy.ssrc.type = (srtp_ssrc_type_t)type;
    policy.ssrc.value = 0;
    policy.key = (uint8_t*)key;
    policy.window_size = 1024;
    policy.allow_repeat_tx = 1;
    policy.next = nullptr;

    if (!_srtp_session)
    {
        int err = srtp_create(&_srtp_session, &policy);
        if (err != srtp_err_status_ok)
        {
            std::cerr << "Failed to create srtp, err: " << err << std::endl;
            _srtp_session = nullptr;
            return false;
        }
        srtp_set_user_data(_srtp_session, this);
    }
    else
    {
        int err = srtp_update(_srtp_session, &policy);
        if (err != srtp_err_status_ok)
        {
            std::cerr << "Failed to update srtp, err: " << err << std::endl;
            return false;
        }
    }

    _rtp_auth_tag_len = policy.rtp.auth_tag_len;
    _rtcp_auth_tag_len = policy.rtcp.auth_tag_len;
    return true;
}

void SrtpSession::handle_event(srtp_event_data_t* ev) 
{
    switch (ev->event) 
    {
    case event_ssrc_collision:
       std::cerr << "SRTP event: ssrc collision" << std::endl;
        break;
    case event_key_soft_limit:
        std::cerr << "SRTP event: reached key soft limit" << std::endl;
        break;
    case event_key_hard_limit:
        std::cerr << "SRTP event: reached key hard limit" << std::endl;
        break;
    case event_packet_index_limit:
        std::cerr << "SRTP event: packet index limit" << std::endl;
        break;
    default:
        std::cerr << "SRTP unknown event: " << ev->event << std::endl;
        break;
    }
}

void SrtpSession::get_auth_tag_len(int* rtp_auth_tag_len, int* rtcp_auth_tag_len)
{
    if (!_srtp_session) 
    {
        std::cerr << "Failed to get auth tag len: no SRTP session" << std::endl;
        return;
    }

    if (rtp_auth_tag_len) 
    {
        *rtp_auth_tag_len = _rtp_auth_tag_len;
    }

    if (rtcp_auth_tag_len) {
        *rtcp_auth_tag_len = _rtcp_auth_tag_len;
    }
}

bool SrtpSession::unprotect_rtp(void* p, int in_len, int* out_len)
{
    if (!_srtp_session)
    {
        std::cerr << "srtp session is null." << std::endl;
        return false;
    }
    *out_len = in_len;
    int err = srtp_unprotect(_srtp_session, p, out_len);
    return err == srtp_err_status_ok;
}

bool SrtpSession::unprotect_rtcp(void* p, int in_len, int* out_len)
{
    if (!_srtp_session)
    {
        std::cerr << "Failed to unprotect rtcp packet: no SRTP session" << std::endl;
        return false;
    }

    *out_len = in_len;
    int err = srtp_unprotect_rtcp(_srtp_session, p, out_len);
    return err == srtp_err_status_ok;
}

bool SrtpSession::protect_rtp(void* p, int in_len, int max_len, int* out_len)
{
    if (!_srtp_session) 
    {
        std::cerr << "Failed to protect rtp packet: no SRTP session";
        return false;
    }

    int need_len = in_len + _rtp_auth_tag_len;
    if (max_len < need_len) 
    {
        std::cerr << "Failed to protect rtp packet: The buffer length "
            << max_len << " is less than needed " << need_len << std::endl;
        return false;
    }

    *out_len = in_len;
    int err = srtp_protect(_srtp_session, p, out_len);
    if (err != srtp_err_status_ok) 
    {
        std::cerr << "Failed to protect rtp packet: err=" << err << std::endl;
        return false;
    }

    return true;
}

bool SrtpSession::protect_rtcp(void* p, int in_len, int max_len, int* out_len)
{
    if (!_srtp_session)
    {
        std::cerr << "Failed to protect rtcp packet: no SRTP session";
        return false;
    }

    int need_len = in_len + _rtcp_auth_tag_len + sizeof(uint32_t);
    if (max_len < need_len) 
    {
        std::cerr << "Failed to protect rtcp packet: The buffer length "
            << max_len << " is less than needed " << need_len;
        return false;
    }

    *out_len = in_len;
    int err = srtp_protect_rtcp(_srtp_session, p, out_len);
    if (err != srtp_err_status_ok) 
    {
        std::cerr << "Failed to protect rtcp packet: err=" << err;
        return false;
    }

    return true;
}