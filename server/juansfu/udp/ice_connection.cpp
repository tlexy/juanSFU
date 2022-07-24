#include "ice_connection.h"

IceConnection::IceConnection(const uvcore::IpAddress& addr)
	:_addr(addr)
{
	_id = _addr.toString();
}

void IceConnection::on_udp_data(uvcore::Udp*)
{}

std::string IceConnection::id() const
{
	return _id;
}