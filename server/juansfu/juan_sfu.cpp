#include <juansfu/juan_sfu.h>
#include <juansfu/signaling/wss_server.h>
#include <juansfu/signaling/signaling_handle.h>

void JuanSfu::init()
{
	SignalingHandle::Instance();
}

void JuanSfu::start_server(int port)
{
	_server = std::make_shared<WssServer>();
	_server->start_timer(5000);
	_server->async_io_start("0.0.0.0", port);
}

void JuanSfu::stop_all()
{
	if (_server)
	{
		_server->stop_io_server();
		_server = nullptr;
	}
}