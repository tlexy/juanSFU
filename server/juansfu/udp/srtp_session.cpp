#include <juansfu/udp/srtp_session.h>
#include <iostream>

SrtpSession::SrtpSession()
{}

void SrtpSession::set_key(int type, int cs, const uint8_t* key, size_t key_len,
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
        return;
    }
    if (!key || key_len != (size_t)policy.rtp.cipher_key_len)
    {
        std::cerr << "SRTP session " << (_srtp_session ? "create" : "update")
            << " failed: invalid key" << std::endl;
        return;
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
            return;
        }
        srtp_set_user_data(_srtp_session, this);
    }
    else
    {
        int err = srtp_update(_srtp_session, &policy);
        if (err != srtp_err_status_ok)
        {
            std::cerr << "Failed to update srtp, err: " << err << std::endl;
            return;
        }
    }

    _rtp_auth_tag_len = policy.rtp.auth_tag_len;
    _rtcp_auth_tag_len = policy.rtcp.auth_tag_len;
}