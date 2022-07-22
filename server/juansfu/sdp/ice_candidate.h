#ifndef ICE_CANDIDATE_H
#define ICE_CANDIDATE_H

#include <stdint.h>
#include <juansfu/rtc_base/socket_address.h>

#define LOCAL_PORT_TYPE "host"

enum IceCandidateComponent {
    RTP = 1,
    RTCP = 2
};

enum IcePriorityValue {
    ICE_TYPE_PREFERENCE_RELAY_UDP = 2,
    ICE_TYPE_PREFERENCE_SRFLX = 100,
    ICE_TYPE_PREFERENCE_PRFLX = 110,
    ICE_TYPE_PREFERENCE_HOST = 126,
};

class IceCandidate
{
public:
    IceCandidate(const std::string ipstr, int port);
    std::string to_string() const;

    std::string compute_foundation(const std::string& type,
        const std::string& protocol,
        const std::string& relay_protocol,
        const rtc::SocketAddress& base);

    uint32_t get_priority(uint32_t type_preference,
        int network_adapter_preference,
        int relay_preference);

public:
    IceCandidateComponent component;
    std::string protocol;
    rtc::SocketAddress address;
    int port = 0;
    uint32_t priority;
    std::string username;
    std::string password;
    std::string type;
    std::string foundation;

};

#endif