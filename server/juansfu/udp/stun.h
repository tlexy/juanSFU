#ifndef JUAN_STUN_PACKET_H
#define JUAN_STUN_PACKET_H

#include <string>
#include <stdint.h>
#include <list>
#include <memory>
#include <juansfu/sdp/session_description.h>
#include <juansfu/rtc_base/byte_buffer.h>
#include <uvnet/core/ip_address.h>

#define STUN_HEADER_SIZE 20
const uint32_t k_stun_magic_cookie = 0x2112A442;

/*
rfc: https://datatracker.ietf.org/doc/html/rfc5389
All STUN messages MUST start with a 20-byte header followed by zero
or more Attributes.  The STUN header contains a STUN message type,
magic cookie, transaction ID, and message length.
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |0 0|     STUN Message Type     |         Message Length        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Magic Cookie                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   |                     Transaction ID (96 bits)                  |
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
typedef enum
{
    STUN_REQUEST = 0,
    STUN_INDICATION = 1,
    STUN_SUCCESS_RESPONSE = 2,
    STUN_ERROR_RESPONSE = 3
} STUN_CLASS_ENUM;

typedef enum
{
    BINDING = 1
} STUN_METHOD_ENUM;

typedef enum
{
    STUN_BINDING = 0x0001,
    STUN_BINDING_RESPONSE = 0x0101
} STUN_MESSAGE_TYPE;

typedef enum
{
    STUN_MAPPED_ADDRESS = 0x0001,
    STUN_USERNAME = 0x0006,
    STUN_MESSAGE_INTEGRITY = 0x0008,
    STUN_ERROR_CODE = 0x0009,
    STUN_UNKNOWN_ATTRIBUTES = 0x000A,
    STUN_REALM = 0x0014,
    STUN_NONCE = 0x0015,
    STUN_XOR_MAPPED_ADDRESS = 0x0020,
    STUN_PRIORITY = 0x0024,
    STUN_USE_CANDIDATE = 0x0025,
    STUN_SOFTWARE = 0x8022,
    STUN_ALTERNATE_SERVER = 0x8023,
    STUN_FINGERPRINT = 0x8028,
    STUN_ICE_CONTROLLED = 0x8029,
    STUN_ICE_CONTROLLING = 0x802A
} STUN_ATTRIBUTE_ENUM;

typedef enum
{
    OK = 0,
    UNAUTHORIZED = 1,
    BAD_REQUEST = 2
} STUN_AUTHENTICATION;

struct stun_header
{
    uint16_t msg_type : 14;
    uint16_t prefix : 2;
    uint16_t msg_len;
    uint32_t msg_cookie;
    uint8_t trans_id[12];
};

class StunAttribute
{
public:
    virtual int write(rtc::ByteBufferWriter* writer) { return 0; };
    virtual ~StunAttribute();
    uint16_t type;
    uint16_t len;
};

class StunAttributeUsername
    : public StunAttribute
{
public:
    std::string remote_ufrag;
    std::string local_ufrag;
};

class StunAttributeIntegrity
    : public StunAttribute
{
public:
    size_t endpos;//计算Integrity的截止位置
    std::string hmac_sha1;
};

class StunAttributeIceControlling
    : public StunAttribute
{
public:
    std::string text;
};

class StunAttributeUseCandidate
    : public StunAttribute
{
};

class StunAttributePriority
    : public StunAttribute
{
public:
    uint32_t priority;
};

class StunAttributeFingerPrint
    : public StunAttribute
{
public:
    uint32_t fp;
};

class StunAttributeXorAddress
    : public StunAttribute
{
public:
    uvcore::IpAddress addr;
    uint8_t family;

    virtual int write(rtc::ByteBufferWriter* writer);
};

class StunPacket
{
public:
    StunPacket();

    void parse_attri(const uint8_t* data, size_t len);
    bool validate(const IceParameter& param, const uint8_t* data, size_t len);

    void add_attribute(std::shared_ptr<StunAttribute>);

    void serialize_bind_response(rtc::ByteBufferWriter* writer, const std::string& pwd);

public:
    static bool is_stun(const uint8_t* data, size_t len);
    static StunPacket* parse(const uint8_t* data, size_t len);
    static bool validate_fingerprint(const uint8_t* data, size_t len);
    static int get_real_len(int len);

private:
    std::shared_ptr<StunAttribute> get_attribute(STUN_ATTRIBUTE_ENUM);

public:
    static const uint8_t magic_cookie[];
    stun_header hdr;
    STUN_CLASS_ENUM cls;
    STUN_METHOD_ENUM method;

private:
    std::list<std::shared_ptr<StunAttribute>> _attris;
};

#endif