#ifndef JUAN_WSS_SERVER_H
#define JUAN_WSS_SERVER_H

#include <core/tcp_server.h>
#include <core/event_loop.h>
#include <tls/ssl_ws_connection.h>
#include <tls/sslws_general_server.h>
#include <unordered_map>

class WssServer : public uvcore::SslWsGeneralServer
{
public:
	WssServer();

protected:
	virtual void on_newconnection(std::shared_ptr<uvcore::SslWsConnection> conn);

	virtual void on_message(std::shared_ptr<uvcore::SslWsConnection> ptr);

	virtual void on_connection_close(std::shared_ptr<uvcore::SslWsConnection> ptr);
	virtual void on_handshake_complete(std::shared_ptr<uvcore::SslWsConnection> ptr);
	virtual void on_websocket_ping(std::shared_ptr<uvcore::SslWsConnection>, const std::string&);
	virtual void on_websocket_close(std::shared_ptr<uvcore::SslWsConnection> ptr, const std::string& text);
	virtual void on_ssl_new(std::shared_ptr<uvcore::SslWsConnection>);
	virtual void timer_event(uvcore::Timer*);

private:
	std::unordered_map<int64_t, std::shared_ptr<uvcore::TcpConnection>> _conn_map;
};

#endif