#include <juansfu/udp/rtc_dtls.h>
#include <juansfu/utils/global.h>
#include <juansfu/api/crypto/crypto_options.h>
#include <iostream>

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
		return rtc::SR_BLOCK;
	}

	return rtc::SR_SUCCESS;
}

rtc::StreamResult DtlsStreamInterface::Write(const void* data,
	size_t data_len,
	size_t* written,
	int* error)
{
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
	size_t written_bytes;
	bool flag = _que.WriteBack(data, len, &written_bytes);

	SignalEvent(this, rtc::SE_READ, 0);
	return flag;
}

/////////////////////////////////DTLS..//////////////////////


RtcDtls::RtcDtls(uvcore::Udp* udp, const uvcore::IpAddress& remote_addr)
{
	auto downward = std::make_unique<DtlsStreamInterface>(udp, remote_addr);
	DtlsStreamInterface* downward_ptr = downward.get();
	_downward = downward_ptr;

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

void RtcDtls::set_remote_dtls(const std::string& alg, const std::string& fp)
{
	if (_remote_fp == fp && !alg.empty())
	{
		std::cerr << "dtls crypto not change." << std::endl;
	}

	_remote_alg = alg;
	_remote_fp = fp;
}

bool RtcDtls::setup_dtls()
{
	if (_dtls_adapter)
	{
		return false;
	}
	_dtls_adapter->SetIdentity(Global::GetInstance()->get_dtls_certificate()->identity()->Clone());
	_dtls_adapter->SetMode(rtc::SSL_MODE_DTLS);
	_dtls_adapter->SetMaxProtocolVersion(rtc::SSL_PROTOCOL_DTLS_12);
	_dtls_adapter->SetServerRole(rtc::SSL_SERVER);

	if (_remote_fp.size() && !_dtls_adapter->SetPeerCertificateDigest(
		_remote_alg,
		(const unsigned char*)_remote_fp.c_str(),
		_remote_fp.size()))
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
}

void RtcDtls::handle_dtls(const uint8_t* data, size_t len)
{}