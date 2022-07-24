#include <juansfu/udp/stun.h>
#include <juansfu/rtc_base/byte_order.h>
#include <juansfu/rtc_base/crc32.h>
#include <iostream>
#include <juansfu/utils/sutil.h>
#include <juansfu/rtc_base/message_digest.h>
#include <string.h>

#define FINGER_PRINT_ATTRI_SIZE 8

const uint32_t STUN_FINGERPRINT_XOR_VALUE = 0x5354554e;
const size_t k_stun_message_integrity_size = 20;

//The magic cookie field MUST contain the fixed value 0x2112A442
const uint8_t StunPacket::magic_cookie[] = { 0x21, 0x12, 0xA4, 0x42 };

StunAttribute::~StunAttribute()
{}

StunPacket::StunPacket()
{}

bool StunPacket::is_stun(const uint8_t* data, size_t len) {
    if ((len >= STUN_HEADER_SIZE) && (data[0] < 3) &&
        (data[4] == StunPacket::magic_cookie[0]) && (data[5] == StunPacket::magic_cookie[1]) &&
        (data[6] == StunPacket::magic_cookie[2]) && (data[7] == StunPacket::magic_cookie[3])) {
        stun_header* hdr = (stun_header*)data;
        if (hdr->prefix == 0)
        {
            return true;
        }
        return false;
    }

    return false;
}

uint16_t read_2bytes(const uint8_t* data) {
    uint16_t value = 0;
    uint8_t* output = (uint8_t*)&value;

    output[1] = *data++;
    output[0] = *data++;

    return value;
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
    /*
   The message type field is decomposed further into the following
   structure:
                 0                 1
                 2  3  4 5 6 7 8 9 0 1 2 3 4 5

                +--+--+-+-+-+-+-+-+-+-+-+-+-+-+
                |M |M |M|M|M|C|M|M|M|C|M|M|M|M|
                |11|10|9|8|7|1|6|5|4|0|3|2|1|0|
                +--+--+-+-+-+-+-+-+-+-+-+-+-+-+

                Figure 3: Format of STUN Message Type Field
*/
    uint16_t msg_class = ((data[0] & 0x01) << 1) | ((data[1] & 0x10) >> 4);
    const uint8_t* p = data;
    uint16_t msg_type = read_2bytes(p);
    uint16_t msg_method = (msg_type & 0x000f) | ((msg_type & 0x00e0) >> 1) | ((msg_type & 0x3E00) >> 2);

    auto ptr = new StunPacket;
    ptr->cls = static_cast<STUN_CLASS_ENUM>(msg_class);
    ptr->method = static_cast<STUN_METHOD_ENUM>(msg_method);
    ptr->parse_attri(data, len);
    return ptr;
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

bool StunPacket::validate(const IceParameter& param, const uint8_t* data, size_t len)
{
    //1. 验证USERNAME中的remote_ufrag
    //2. 消息完整性验证
    for (auto it = _attris.begin(); it != _attris.end(); ++it)
    {
        if ((*it)->type == STUN_USERNAME)
        {
            auto ptr = std::dynamic_pointer_cast<StunAttributeUsername>(*it);
            if (ptr && ptr->remote_ufrag != param.ufrag)
            {
                return false;
            }
        }
        else if ((*it)->type == STUN_MESSAGE_INTEGRITY)
        {
            auto ptr = std::dynamic_pointer_cast<StunAttributeIntegrity>(*it);
            if (!ptr)
            {
                return false;
            }
            stun_header* hdr = (stun_header*)data;
            //修改长度，长度应该包括Integrity的长度（24），但计算的内容不包括Integrity的内容
            uint16_t ori_msg_len = hdr->msg_len;
            hdr->msg_len = rtc::HostToNetwork16(ptr->endpos + 24 - STUN_HEADER_SIZE);
            char hmac[k_stun_message_integrity_size];
            size_t ret = rtc::ComputeHmac(rtc::DIGEST_SHA_1, param.passwd.c_str(),
                param.passwd.length(), data, ptr->endpos, hmac,
                sizeof(hmac));

            if (ret != k_stun_message_integrity_size) {
                return false;
            }
            if (memcmp(hmac, ptr->hmac_sha1.c_str(), k_stun_message_integrity_size) != 0)
            {
                return false;
            }
            hdr->msg_len = ori_msg_len;
        }
    }
    return true;
}

void StunPacket::parse_attri(const uint8_t* data, size_t len)
{
    const uint8_t* attri_start = data + STUN_HEADER_SIZE;

    int counter = 0;
    size_t pos = 0;
    //first 4: attribute header size
    while (pos + 4 + STUN_HEADER_SIZE < len && counter < 20)
    {
        uint16_t type = rtc::GetBE16(attri_start + pos);
        uint16_t flen = rtc::GetBE16(attri_start + pos + sizeof(type));
        ++counter;
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
            //指针移动
            pos += (real_len + 4); //内容长度占用+头部长度
            std::vector<std::string> vecs;
            SUtil::split(str, ":", vecs);
            if (vecs.size() == 2)
            {
                ptr->local_ufrag = vecs[1];
                ptr->remote_ufrag = vecs[0];
            }
            _attris.push_back(ptr);
        }
        else if (type == STUN_MESSAGE_INTEGRITY)
        {
            auto ptr = std::make_shared<StunAttributeIntegrity>();
            ptr->endpos = pos + STUN_HEADER_SIZE;
            ptr->len = flen;
            ptr->type = type;
            //sha1的长度固定为20，是4的倍数
            ptr->hmac_sha1 = std::string((const char*)attri_start + pos + 4, flen);
            pos += (ptr->len + 4);
            _attris.push_back(ptr);
        }
        else
        {
            //未知属性
            uint16_t real_len = flen;
            if (flen % 4 != 0)
            {
                uint16_t ret = flen / 4;
                real_len = (ret + 1) * 4;
            }
            //指针移动
            pos += (real_len + 4); //内容长度占用+头部长度
        }
    }
}