#include <juansfu/signaling/room_member.h>
#include <juansfu/udp/udp_receiver.h>
#include <juansfu/utils/global.h>
#include <juansfu/udp/stun.h>
#include <juansfu/udp/ice_connection.h>
#include <juansfu/udp/rtc_dtls.h>
#include <juansfu/signaling/port_mgr.h>
#include <juansfu/udp/rtc_dtls.h>
#include <juansfu/udp/srtp_transport.h>
#include <juansfu/rtprtcp/rtprtcp_pub.hpp>
#include <juansfu/udp/rtc_stream_mgr.h>
#include <iostream>

void RoomMember::start_recv(const uvcore::IpAddress& addr)
{
	using namespace std::placeholders;

	_addr = addr;
	_addr.parse();
	udp_receiver = std::make_shared<UdpReceiver>(addr, Global::GetInstance()->get_udp_server());
	udp_receiver->set_data_cb(std::bind(&RoomMember::on_udp_receive, this, _1, _2));
	udp_receiver->start();
}

void RoomMember::stop_recv()
{
	if (udp_receiver)
	{
		udp_receiver->stop();
		int port = _addr.getPort();
		UdpPortManager::GetInstance()->recycle_port(port);
		udp_receiver = nullptr;
	}
}

void RoomMember::destory()
{
	/*for (auto it = dtls_connections.begin(); it != dtls_connections.end(); ++it)
	{
		it->second->destory();
	}*/
	for (auto it = udp_handles.begin(); it != udp_handles.end(); ++it)
	{
		if (it->second->dtls_connection)
		{
			it->second->dtls_connection->destory();
		}
	}
}

void RoomMember::on_udp_receive(uvcore::Udp* udp, const uvcore::IpAddress& addr)
{
	std::shared_ptr<MemberUdpPorts> up = nullptr;
	auto it = udp_handles.find(addr.toString());
	if (it == udp_handles.end())
	{
		up = std::make_shared<MemberUdpPorts>();
		udp_handles[addr.toString()] = up;
	}
	else
	{
		up = it->second;
	}
	if (StunPacket::is_stun(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size()))
	{
		//std::cout << "stun from: " << addr.toString() << std::endl;
		if (up->ice_connection)
		{
			up->ice_connection->on_udp_data(udp);
			return;
		}
		//new connection...
		StunPacket* sp = StunPacket::parse(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size());
		udp->get_inner_buffer()->reset();

		bool flag = true;
		if (answer_sdp->media_contents.size() > 0)
		{
			flag = sp->validate(answer_sdp->media_contents[0]->ice,
				udp->get_inner_buffer()->read_ptr(),
				udp->get_inner_buffer()->readable_size());
		}
		if (flag)
		{
			auto ptr = std::make_shared<IceConnection>(addr, _addr);
			ptr->set_ice_pwd(answer_sdp->media_contents[0]->ice.passwd);
			up->ice_connection = ptr;
			ptr->send_binding_response(udp, sp);
		}
		delete sp;
	}
	else if (is_rtcp(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size())
		|| is_rtp(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size()))
	{
		if (!up->dtls_connection)
		{
			std::cerr << "dtls_connection not exist." << std::endl;
			udp->get_inner_buffer()->reset();
			return;
		}
		//std::cerr << "is_rtcp protocol." << std::endl;
		if (!up->srtp)
		{
			up->srtp = std::make_shared<SrtpTransport>(udp, addr);
			std::vector<int> send_extension_ids;
			std::vector<int> recv_extension_ids;
			int selected_crypto_suite;
			std::string send_key;
			std::string recv_key;
			bool flag = up->dtls_connection->extract_srtp_keys(&selected_crypto_suite, send_key, recv_key);
			if (flag)
			{
				std::cout << "extract_srtp_keys, cs: " << selected_crypto_suite
					<< "\tsend_key: " << send_key.size() << "\trecv_key: " << recv_key.size() << std::endl;
				up->srtp->set_srtp_param(selected_crypto_suite, (uint8_t*)send_key.c_str(), send_key.size(),
					send_extension_ids, selected_crypto_suite, (uint8_t*)recv_key.c_str(), recv_key.size(),
					recv_extension_ids);
			}
			else
			{
				std::cerr << "dtls_connection extract_srtp_keys failed." << std::endl;
				udp->get_inner_buffer()->reset();
				return;
			}
			
		}
		RtcStreamMgr::GetInstance()->add_stream(uid, addr.toString(), up->srtp);
		if (is_rtcp(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size()))
		{
			up->srtp->handle_rtcp_data(udp);
		}
		else
		{
			up->srtp->handle_rtp_data(udp);
		}
	}
	else if (RtcDtls::is_dtls(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size()))
	{
		//auto it = dtls_connections.find(addr.toString());
		if (up->dtls_connection == nullptr)
		{
			if (offer_sdp->media_contents[0]->dtls->identity_fp)
			{
				auto ptr = std::make_shared<RtcDtls>(udp, addr);
				ptr->set_remote_dtls(offer_sdp->media_contents[0]->dtls->identity_fp->algorithm,
					offer_sdp->media_contents[0]->dtls->identity_fp->digest.cdata(),
					offer_sdp->media_contents[0]->dtls->identity_fp->digest.size());
				bool flag = ptr->setup_dtls();
				if (flag)
				{
					up->dtls_connection = ptr;
					//dtls_connections[addr.toString()] = ptr;
					//it = dtls_connections.find(addr.toString());
				}
				else
				{
					std::cerr << "dtls setup failed." << std::endl;
				}
			}
			else
			{
				std::cerr << "remote digest is empty." << std::endl;
			}
		}
		/*if (it != dtls_connections.end())
		{
			it->second->handle_dtls(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size());
		}*/
		if (up->dtls_connection)
		{
			up->dtls_connection->handle_dtls(udp->get_inner_buffer()->read_ptr(), udp->get_inner_buffer()->readable_size());
		}
	}
	else
	{
		std::cerr << "unknown protocol." << std::endl;
	}
	udp->get_inner_buffer()->reset();
}