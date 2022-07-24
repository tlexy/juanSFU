#include "ice_connection.h"
#include <juansfu/udp/stun.h>

IceConnection::IceConnection(const uvcore::IpAddress& remote_addr, const uvcore::IpAddress& local_addr)
	:_remote_addr(remote_addr),
	_local_addr(local_addr)
{
	_id = _remote_addr.toString();
}

void IceConnection::on_udp_data(uvcore::Udp*)
{}

void IceConnection::send_binding_response(uvcore::Udp* udp, StunPacket* sp, const std::string& pwd)
{
	//三个属性：STUN_XOR_MAPPED_ADDRESS， STUN_MESSAGE_INTEGRITY， STUN_FINGERPRINT
	StunPacket* retsp = new StunPacket;
	retsp->hdr.msg_type = STUN_BINDING_RESPONSE;
	retsp->hdr.prefix = 0;
	retsp->hdr.msg_cookie = rtc::HostToNetwork32(k_stun_magic_cookie);
	memcpy(retsp->hdr.trans_id, sp->hdr.trans_id, sizeof(sp->hdr.trans_id));
	retsp->hdr.msg_len = 0;

	/*0x01:IPv4
	0x02 : IPv6*/
	auto pptr = std::make_shared<StunAttributeXorAddress>();
	pptr->family = 0x01;
	pptr->len = 8;
	pptr->type = STUN_XOR_MAPPED_ADDRESS;
	pptr->addr = _local_addr;
	retsp->add_attribute(pptr);

	rtc::ByteBufferWriter* writer = new rtc::ByteBufferWriter();
	retsp->serialize_bind_response(writer, pwd);
	udp->send2(writer->Data(), writer->Length(), _remote_addr);
}

std::string IceConnection::id() const
{
	return _id;
}