#ifndef JUAN_SFU_SERVER_H
#define JUAN_SFU_SERVER_H

#include <juansfu/utils/singleton.h>
#include <memory>

class WssServer;

class JuanSfu : public Singleton<JuanSfu>
{
public:
	void init(const char* config_file);

	void start_server();
	void stop_all();

private:
	std::shared_ptr<WssServer> _server;
	int _port;
};

#endif