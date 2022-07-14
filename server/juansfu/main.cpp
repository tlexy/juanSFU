#include <iostream>
#include <tls/tls_config.h>
#include <juansfu/juan_sfu.h>

#include <utils/sock_utils.h>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Iphlpapi.lib")
#pragma comment (lib, "Psapi.lib")
#pragma comment (lib, "Userenv.lib")

int main(int argc, char* argv[])
{
	sockets::Init();

	TlsConfig::init_server("server.crt", "server.key");

	JuanSfu::Instance();

	sockets::Destroy();

	return 0;
}