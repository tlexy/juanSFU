#include <juansfu/udp/stun.h>
#include <juansfu/rtc_base/byte_order.h>
#include <juansfu/rtc_base/crc32.h>
#include <iostream>
#include <juansfu/utils/sutil.h>

#define FINGER_PRINT_ATTRI_SIZE 8

const uint32_t STUN_FINGERPRINT_XOR_VALUE = 0x5354554e;

//The magic cookie field MUST contain the fixed value 0x2112A442
const uint8_t StunPacket::magic_cookie[] = { 0x21, 0x12, 0xA4, 0x42 };

StunPacket::StunPacket()
{}

bool StunPacket::is_stun(const uint8_t* data, size_t len) {
    if ((len >= STUN_HEADER_SIZE) && (data[0] < 3) &&
        (data[4] == StunPacket::magic_cookie[0]) && (data[5] == StunPacket::magic_cookie[1]) &&
        (data[6] == StunPacket::magic_cookie[2]) && (data[7] == StunPacket::magic_cookie[3])) {
        return true;
    }

    return false;
}

StunPacket* StunPacket::parse(const uint8_t* data, size_t len)
{
    if (!StunPacket::is_stun(data, len))
    {
        return nullptr;
    }
    stun_header* hdr = (stun_header*)data;
    uint16_t msg_len = rtc::NetworkToHost16(hdr->msg_len);
    if (msg_len + STUN_HEADER_SIZE != len
        || ((msg_len & 0x03) != 0))
    {
        return nullptr;
    }

    bool flag = validate_fingerprint(data, len);
    std::cout << "validate_fingerprint: " << flag << std::endl;
    if (!flag)
    {
        return nullptr;
    }
    auto ptr = new StunPacket;
    ptr->parse_attri(data, len);
    return nullptr;
}

/*
* 0 1 2 3
0  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Type                          | Length |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Value (variable) ....
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

bool StunPacket::validate_fingerprint(const uint8_t* data, size_t len)
{
    const uint8_t* finger_print = (const uint8_t*)(data + len - FINGER_PRINT_ATTRI_SIZE);
    uint16_t type = rtc::GetBE16(finger_print);
    uint16_t flen = rtc::GetBE16(finger_print + sizeof(type));
    uint32_t fp = rtc::GetBE32(finger_print + 4);

    if (type != STUN_FINGERPRINT)
    {
        return true;
    }
    if (flen != 4)
    {
        return false;
    }
    uint32_t crc32 = rtc::ComputeCrc32(data, len - FINGER_PRINT_ATTRI_SIZE);
    uint32_t crc2 = crc32 ^ STUN_FINGERPRINT_XOR_VALUE;
    return fp == crc2;
}

void StunPacket::parse_attri(const uint8_t* data, size_t len)
{
    const uint8_t* attri_start = data + STUN_HEADER_SIZE;

    size_t pos = 0;
    //first 4: attribute header size
    while (pos + 4 + STUN_HEADER_SIZE < len)
    {
        uint16_t type = rtc::GetBE16(attri_start + pos);
        uint16_t flen = rtc::GetBE16(attri_start + pos + sizeof(type));

        if (type == STUN_USERNAME)
        {
            auto ptr = std::make_shared<StunAttributeUsername>();
            ptr->len = flen;
            ptr->type = type;
            std::string str = std::string((const char*)attri_start + pos + 4, flen);
            uint16_t real_len = flen;
            if (flen % 4 != 0)
            {
                uint16_t ret = flen / 4;
                real_len = (ret + 1) * 4;
            }
            pos += real_len + 4;
            std::vector<std::string> vecs;
            SUtil::split(str, ":", vecs);
            if (vecs.size() == 2)
            {
                ptr->remote_ufrag = vecs[0];
                ptr->local_ufrag = vecs[1];
            }
            _attris.push_back(ptr);
        }
        else
        {
            //未知属性
        }
    }
}