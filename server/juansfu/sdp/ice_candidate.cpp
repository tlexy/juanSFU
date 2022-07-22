#include <juansfu/sdp/ice_candidate.h>
#include <sstream>
#include <juansfu/rtc_base/crc32.h>

IceCandidate::IceCandidate(const std::string ipstr, int port_)
    :protocol("udp"),
    address(ipstr, port_),
    port(port_)
{

}

/*
priority = (2^24)*(type preference) +
           (2^8)*(local preference) +
           (2^0)*(256 - component ID)
*/
// 0	1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  NIC Pref     |    Addr Pref  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// NIC type：3G/wifi
// Addr pref: 定义在RFC3484
uint32_t IceCandidate::get_priority(uint32_t type_preference,
    int network_adapter_preference,
    int relay_preference)
{
    int addr_ref = rtc::IPAddressPrecedence(address.ipaddr());
    int local_pref = ((network_adapter_preference << 8) | addr_ref) + relay_preference;
    return (type_preference << 24) | (local_pref << 8) | (256 - (int)component);
}

std::string IceCandidate::compute_foundation(const std::string& type,
    const std::string& protocol,
    const std::string& relay_protocol,
    const rtc::SocketAddress& base)
{
    std::stringstream ss;
    ss << type << base.HostAsURIString() << protocol << relay_protocol;
    return std::to_string(rtc::ComputeCrc32(ss.str()));
}

std::string IceCandidate::to_string() const
{
    return std::string();
}