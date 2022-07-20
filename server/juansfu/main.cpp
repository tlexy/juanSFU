#include <iostream>
#include <thread>
#include <signal.h>
//#include <tls/tls_config.h>
#include <juansfu/juan_sfu.h>
#include <utils/sock_utils.h>
#include <juansfu/rtc_base/helpers.h>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Iphlpapi.lib")
#pragma comment (lib, "Psapi.lib")
#pragma comment (lib, "Userenv.lib")

static bool gstop = false;

void kill_signal(int signal)
{
	JuanSfu::GetInstance()->stop_all();
	std::cout << "kill process signal ..." << std::endl;
	gstop = true;
}

int main(int argc, char* argv[])
{
	sockets::Init();

	signal(SIGINT, kill_signal);
	signal(SIGTERM, kill_signal);
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

#if defined(_WIN32) || defined(_WIN64)
	signal(SIGBREAK, kill_signal);
#endif

	//TlsConfig::init_server("server.crt", "server.key");
	rtc::InitRandom(time(NULL));
	std::cout << "random str24: " << rtc::CreateRandomString(24) << std::endl;

	JuanSfu::Instance();
	JuanSfu::GetInstance()->init();
	JuanSfu::GetInstance()->start_server(5000);

	while (!gstop)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	sockets::Destroy();

	return 0;
}