#ifndef JUAN_ICE_CONNECTION_H
#define JUAN_ICE_CONNECTION_H

#include <uvnet/core/udp_server.h>
#include <uvnet/core/ip_address.h>

class StunPacket;

class IceConnection
{
public:
	IceConnection(const uvcore::IpAddress& remote_addr, const uvcore::IpAddress& local_addr);
	void set_ice_pwd(const std::string&);
	void on_udp_data(uvcore::Udp*);
	std::string id() const;

	void send_binding_response(uvcore::Udp*, StunPacket*);

private:
	uvcore::IpAddress _remote_addr;
	uvcore::IpAddress _local_addr;
	std::string _id;
	std::string _ice_pwd;
};

#endif