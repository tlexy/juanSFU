#include <juansfu/udp/rtc_dtls.h>
#include <juansfu/utils/global.h>
#include <juansfu/api/crypto/crypto_options.h>
#include <iostream>

const size_t k_dtls_record_header_len = 13;
const size_t k_max_dtls_packet_len = 2048;

DtlsStreamInterface::DtlsStreamInterface(uvcore::Udp* udp, const uvcore::IpAddress& remote_addr)
	:_que(2048, 2),
	_udp(udp),
	_remote_addr(remote_addr)
{}

rtc::StreamState DtlsStreamInterface::GetState() const
{
	return _state;
}

rtc::StreamResult DtlsStreamInterface::Read(void* buffer,
	size_t buffer_len,
	size_t* read,
	int* error)
{
	if (_state == rtc::SS_CLOSED) {
		return rtc::SR_EOS;
	}

	if (_state == rtc::SS_OPENING) {
		return rtc::SR_BLOCK;
	}

	if (!_que.ReadFront(buffer, buffer_len, read)) {
		std::cerr << std::endl << "read front failed." << std::endl;
		return rtc::SR_BLOCK;
	}
	std::cerr << std::endl << "read front, bytes: " << *read << std::endl;
	return rtc::SR_SUCCESS;
}

rtc::StreamResult DtlsStreamInterface::Write(const void* data,
	size_t data_len,
	size_t* written,
	int* error)
{
	std::cout << "dtls write data, len: " << data_len << std::endl;
	_udp->send2((const char*)data, data_len, _remote_addr);
	if (written) {
		*written = data_len;
	}

	return rtc::SR_SUCCESS;
}

void DtlsStreamInterface::Close()
{
	_state = rtc::SS_CLOSED;
	_que.Clear();
}

bool DtlsStreamInterface::on_packet_receive(const uint8_t* data, size_t len)
{
	//size_t written_bytes = 0;;
	bool flag = _que.WriteBack(data, len, NULL);
	if (!flag)
	{
		std::cerr << "write to dtls error." << std::endl;
	}
	else
	{
		std::cout << std::endl << "write udp to dtls buffer, len: " << len << std::endl;
	}
	SignalEvent(this, rtc::SE_READ, 0);
	return flag;
}

/////////////////////////////////DTLS..//////////////////////


RtcDtls::RtcDtls(uvcore::Udp* udp, const uvcore::IpAddress& remote_addr)
{
	auto downward = std::make_unique<DtlsStreamInterface>(udp, remote_addr);
	DtlsStreamInterface* downward_ptr = downward.get();
	_downward = downward_ptr;

	rtc::ThreadManager::Instance()->WrapCurrentThread();
	_dtls_adapter = rtc::SSLStreamAdapter::Create(std::move(downward));
	webrtc::CryptoOptions crypto_options;
	_srtp_ciphers = crypto_options.GetSupportedDtlsSrtpCryptoSuites();
}

bool RtcDtls::is_dtls(const uint8_t* data, size_t len)
{
	//https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
	return ((len >= 13) && (data[0] > 19 && data[0] < 64));
}

bool RtcDtls::is_dtls_client_hello_packet(const uint8_t* data, size_t len)
{
	if (!is_dtls(data, len)) {
		return false;
	}

	//const uint8_t* u = reinterpret_cast<const uint8_t*>(buf);
	return len > 17 && (data[0] == 22 && data[13] == 1);
}

void RtcDtls::set_remote_dtls(const std::string& alg, const uint8_t* digest, size_t digest_len)
{
	rtc::Buffer remote_fingerprint_value(digest, digest_len);
	if (_remote_fingerprint_value == remote_fingerprint_value && !alg.empty())
	{
		std::cerr << "dtls crypto not change." << std::endl;
		return;
	}

	_remote_alg = alg;
	_remote_fingerprint_value = std::move(remote_fingerprint_value);
}

bool RtcDtls::setup_dtls()
{
	if (!_dtls_adapter)
	{
		return false;
	}

	_dtls_adapter->SetIdentity(Global::GetInstance()->get_dtls_certificate()->identity()->Clone());
	_dtls_adapter->SetMode(rtc::SSL_MODE_DTLS);
	_dtls_adapter->SetMaxProtocolVersion(rtc::SSL_PROTOCOL_DTLS_12);
	_dtls_adapter->SetServerRole(rtc::SSL_SERVER);
	_dtls_adapter->SignalEvent.connect(this, &RtcDtls::slot_dtls_event);
	_dtls_adapter->SignalSSLHandshakeError.connect(this, &RtcDtls::slot_dtls_handshake_error);

	if (_remote_fingerprint_value.size() < 1)
	{
		return false;
	}
	rtc::SSLPeerCertificateDigestError err;
	if (!_dtls_adapter->SetPeerCertificateDigest(
		_remote_alg,
		(const unsigned char*)_remote_fingerprint_value.data(),
		_remote_fingerprint_value.size(), &err))
	{
		return false;
	}

	if (!_srtp_ciphers.empty()) 
	{
		if (!_dtls_adapter->SetDtlsSrtpCryptoSuites(_srtp_ciphers)) 
		{
			std::cerr << ": Failed to set DTLS-SRTP crypto suites" << std::endl;
			return false;
		}
	}
	else 
	{
		std::cerr << ": Not using DTLS-SRTP" << std::endl;
		return false;
	}
	return true;
}

void RtcDtls::slot_dtls_event(rtc::StreamInterface* dtls, int sig, int error)
{
	if (sig & rtc::SE_OPEN) 
	{
		std::cerr << ": DTLS handshake complete." << std::endl;;
		_state = DtlsTransportState::k_connected;
	}
	if (sig & rtc::SE_CLOSE)
	{
		if (!error) 
		{
			std::cerr << ": DTLS transport closed" << std::endl;
			_state = DtlsTransportState::k_closed;
		}
		else 
		{
			std::cerr << ": DTLS transport closed with error, "
				<< "code=" << error << std::endl;
			
			_state = DtlsTransportState::k_failed;
		}
	}
	//
	if (sig & rtc::SE_READ) 
	{
		char buf[k_max_dtls_packet_len];
		size_t read;
		int read_error;
		rtc::StreamResult ret;
		// 因为一个数据包可能会包含多个DTLS record，需要循环读取
		do {
			ret = _dtls_adapter->Read(buf, sizeof(buf), &read, &read_error);
			if (ret == rtc::SR_SUCCESS) 
			{
			}
			else if (ret == rtc::SR_EOS) 
			{
				std::cerr << ": DTLS transport closed by remote." << std::endl;
				_state = DtlsTransportState::k_closed;
				//signal_closed(this);
			}
			else if (ret == rtc::SR_ERROR) 
			{
				std::cerr << ": Closed DTLS transport by remote "
					<< "with error, code=" << read_error << std::endl;
				_state = DtlsTransportState::k_failed;
				//signal_closed(this);
			}

		} while (ret == rtc::SR_SUCCESS);
	}
}

void RtcDtls::slot_dtls_handshake_error(rtc::SSLHandshakeError err) 
{
	std::cerr << ": DTLS handshake error=" << (int)err << std::endl;
}

void RtcDtls::handle_dtls(const uint8_t* data, size_t len)
{
	if (_state == DtlsTransportState::k_new)
	{
		int ret = _dtls_adapter->StartSSL();
		if (ret == 0)
		{
			_state = DtlsTransportState::k_connecting;
		}
		else
		{
			std::cerr << "start ssl failed." << std::endl;
		}
	}
	if (_state == DtlsTransportState::k_connecting)
	{
		if (is_dtls_client_hello_packet(data, len))
		{
			bool flag = do_handle_dtls_packet(data, len);
			if (flag)
			{
				_state = DtlsTransportState::k_connected;
			}
			else
			{
				std::cerr << "dtls handle hello failed." << std::endl;
			}
		}
	}
	else if (_state == DtlsTransportState::k_connected)
	{
		bool flag = do_handle_dtls_packet(data, len);
		if (!flag)
		{
			std::cerr << "dtls handle packet failed." << std::endl;
		}
	}

}

bool RtcDtls::do_handle_dtls_packet(const uint8_t* data, size_t len)
{
	size_t tmp_size = len;
	const uint8_t* temp = data;
	while (tmp_size > 0) {
		if (tmp_size < k_dtls_record_header_len) {
			return false;
		}

		size_t record_len = (data[11] << 8) | data[12];
		if (record_len + k_dtls_record_header_len > tmp_size) {
			return false;
		}

		data += k_dtls_record_header_len + record_len;
		tmp_size -= k_dtls_record_header_len + record_len;
	}
	//std::cout << ""
	return _downward->on_packet_receive(temp, len);
}