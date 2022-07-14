#ifndef JUAN_SFU_SERVER_H
#define JUAN_SFU_SERVER_H

#include <juansfu/utils/singleton.h>
#include <memory>

class WssServer;

class JuanSfu : public Singleton<JuanSfu>
{
public:
	void init();

	void start_server(int port);
	void stop_all();

private:
	std::shared_ptr<WssServer> _server;
};

#endif