#include "wss_server.h"
#include <iostream>
#include <juansfu/utils/sutil.h>
#include <juansfu/signaling/signaling_handle.h>

WssServer::WssServer()
{}

void WssServer::on_newconnection(std::shared_ptr<uvcore::SslWsConnection> conn)
{
	std::cout << "new connection here, id : " << conn->id() << std::endl;
	_conn_map[conn->id()] = conn;
}

void WssServer::on_message(std::shared_ptr<uvcore::SslWsConnection> ptr)
{
	if (ptr->error() != 0)
	{
		ptr->close();
		return;
	}
	std::string recv_msg((char*)ptr->get_dec_buffer()->read_ptr(), ptr->get_dec_buffer()->readable_size());
	std::cout << "recv: " << recv_msg.c_str() << std::endl;
	ptr->get_dec_buffer()->reset();
	
	Json::Value json;
	bool flag = SUtil::parseJson(recv_msg.c_str(), json);
	if (flag)
	{
		SignalingHandle::GetInstance()->handle(json, ptr);
	}
	else
	{
		std::cerr << "recv msg parse to json failed." << std::endl;
	}
}

void WssServer::on_connection_close(std::shared_ptr<uvcore::SslWsConnection> ptr)
{
	std::cout << "connection close." << std::endl;
	_conn_map.erase(ptr->id());
}

void WssServer::on_handshake_complete(std::shared_ptr<uvcore::SslWsConnection> ptr)
{
	std::cout << "websocket handshake done." << std::endl;
}

void WssServer::on_websocket_ping(std::shared_ptr<uvcore::SslWsConnection>, const std::string&)
{
	std::cout << "websocket recv ping." << std::endl;
}

void WssServer::on_websocket_close(std::shared_ptr<uvcore::SslWsConnection> ptr, const std::string& text)
{
	std::cout << "websocket close, text: " << text.c_str() << std::endl;
}

void WssServer::on_ssl_new(std::shared_ptr<uvcore::SslWsConnection>)
{
	std::cout << "ssl handshake " << std::endl;
}

void WssServer::timer_event(uvcore::Timer*)
{
	//std::cout << "Echo server timer" << std::endl;
}