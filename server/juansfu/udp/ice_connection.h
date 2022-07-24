#ifndef JUAN_ICE_CONNECTION_H
#define JUAN_ICE_CONNECTION_H

#include <uvnet/core/udp_server.h>
#include <uvnet/core/ip_address.h>

class IceConnection
{
public:
	IceConnection(const uvcore::IpAddress&);
	
	void on_udp_data(uvcore::Udp*);
	std::string id() const;

private:
	uvcore::IpAddress _addr;
	std::string _id;
};

#endif