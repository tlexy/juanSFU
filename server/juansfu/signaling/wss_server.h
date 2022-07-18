#ifndef JUAN_WSS_SERVER_H
#define JUAN_WSS_SERVER_H

#include <core/tcp_server.h>
#include <core/event_loop.h>
#include <websocket/ws_connection.h>
#include <websocket/ws_general_server.h>
#include <unordered_map>

class WssServer : public uvcore::WsGeneralServer
{
public:
	WssServer();

protected:
	virtual void on_newconnection(std::shared_ptr<uvcore::WsConnection> conn);

	virtual void on_message(std::shared_ptr<uvcore::WsConnection> ptr);

	virtual void on_connection_close(std::shared_ptr<uvcore::WsConnection> ptr);
	virtual void on_handshake_complete(std::shared_ptr<uvcore::WsConnection> ptr);
	virtual void on_websocket_ping(std::shared_ptr<uvcore::WsConnection>, const std::string&);
	virtual void on_websocket_close(std::shared_ptr<uvcore::WsConnection> ptr, const std::string& text);
	virtual void timer_event(uvcore::Timer*);

private:
	std::unordered_map<int64_t, std::shared_ptr<uvcore::TcpConnection>> _conn_map;
};

#endif