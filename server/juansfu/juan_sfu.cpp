#include <juansfu/juan_sfu.h>
#include <juansfu/signaling/wss_server.h>
#include <juansfu/signaling/signaling_handle.h>
#include <juansfu/signaling/port_mgr.h>
#include <juansfu/utils/sutil.h>
#include <assert.h>
#include <juansfu/utils/global.h>

void JuanSfu::init(const char* config_file)
{
	SignalingHandle::Instance();
	UdpPortManager::Instance();

	Json::Value json = Json::nullValue;
	bool flag = SUtil::readJson(config_file, json);
	assert(flag);

	int port_start = GET_JSON_INT(json, "udp_port_start", 0);
	int port_end = GET_JSON_INT(json, "udp_port_end", 0);
	int step = GET_JSON_INT(json, "udp_port_step", 0);
	int port = GET_JSON_INT(json, "server_port", 0);
	std::string ipstr = GET_JSON_STRING(json, "server_ip", "");

	assert(port_end > port_start);
	assert(step > 0);
	assert(port > 0);
	assert(ipstr.size() > 5);
	UdpPortManager::GetInstance()->init(port_start, port_end, step);
	UdpPortManager::GetInstance()->ipstr = ipstr;
	_port = port;
}

void JuanSfu::start_server()
{
	_server = std::make_shared<WssServer>();
	Global::GetInstance()->init(_server->get_loop());
	_server->start_timer(5000);
	_server->async_io_start("0.0.0.0", _port);
}

void JuanSfu::stop_all()
{
	if (_server)
	{
		_server->stop_io_server();
		_server = nullptr;
	}
}