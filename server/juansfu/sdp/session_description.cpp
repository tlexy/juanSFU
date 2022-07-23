﻿#include <juansfu/sdp/session_description.h>
#include <iostream>
#include <juansfu/utils/sutil.h>
#include <juansfu/rtc_base/helpers.h>
#include <juansfu/utils/global.h>

const static std::string k_media_proto_dtls_savpf = "UDP/TLS/RTP/SAVPF";
const static std::string k_media_proto_savpf = "RTP/SAVPF";

SessionDescription::SessionDescription()
{
	_video_sup.name = "video";
	_audio_sup.name = "audio";
}

std::string SessionDescription::to_string()
{
	std::stringstream ss;
	ss << "v=" << session.version << "\r\n";
	ss << "o=" << session.username << " " << session.session_id << " " << session.session_version
		<< " " << session.nett_type << " " << session.addr_type << " " << session.uni_addr << "\r\n";
	ss << "s=" << session.session_name << "\r\n";
	ss << "t=" << session.ts << "\r\n";
	ss << session.attr << "\r\n";

	ss << "a=group:BUNDLE 0 1" << "\r\n";

	for (int i = 0; i < media_contents.size(); ++i)
	{
		add_media_content(ss, media_contents[i]);
	}

	return ss.str();
}

void SessionDescription::add_media_content(std::stringstream& ss, std::shared_ptr<MediaContent> ptr)
{
	ss << "m=" << ptr->name() << " 9 ";
	if (ptr->use_dtls)
	{
		ss << k_media_proto_dtls_savpf;
	}
	else
	{
		ss << k_media_proto_savpf;
	}
	for (int i = 0; i < ptr->rtpmaps.size(); ++i)
	{
		ss << " " << ptr->rtpmaps[i].payload_type;
	}
	ss << "\r\n";
	ss << "c=" << ptr->connection_info << "\r\n";
	ss << "a=" << ptr->fix_attr1 << "\r\n";
	for (int i = 0; i < ptr->rtpmaps.size(); ++i)
	{
		ss << "a=rtpmap:" << ptr->rtpmaps[i].payload_type << " " << ptr->rtpmaps[i].desc << "\r\n";
	}
	//
	for (int i = 0; i < ptr->fmtps.size(); ++i)
	{
		ss << "a=fmtp:" << ptr->fmtps[i].codec_id << " ";
		for (auto it = ptr->fmtps[i].params.begin(); it != ptr->fmtps[i].params.end();)
		{
			ss << it->first << "=" << it->second;
			if (++it != ptr->fmtps[i].params.end())
			{
				ss << ",";
			}
			else
			{
				break;
			}
		}
		ss << "\r\n";
	}
	//
	for (int i = 0; i < ptr->rtpmaps.size(); ++i)
	{
		for (int j = 0; j < ptr->rtpmaps[i].fbs.size(); ++j)
		{
			ss << "a=rtcp-fb:" << ptr->rtpmaps[i].payload_type << " " << ptr->rtpmaps[i].fbs[j]->id;
			if (ptr->rtpmaps[i].fbs[j]->param.empty())
			{
				ss << "\r\n";
			}
			else
			{
				ss << " " << ptr->rtpmaps[i].fbs[j]->param << "\r\n";
			}
		}
	}
	//
	if (ptr->rtcp_mux)
	{
		ss << "a=rtcp-mux\r\n";
	}
	ss << "a=" << ptr->ice.mode << "\r\n";
	ss << "a=ice-ufrag:" << ptr->ice.ufrag << "\r\n";
	ss << "a=ice-pwd:" << ptr->ice.passwd << "\r\n";
	if (_fingerprint && ptr->use_dtls)
	{
		auto fp = _fingerprint.get();
		ss << "a=fingerprint:" << fp->algorithm << " " << fp->GetRfc4572Fingerprint() << "\r\n";
		ss << "a=setup:" << ptr->connection_role << "\r\n";
	}
	ss << "a=mid:" << ptr->mid() << "\r\n";
	add_media_direction(ss, ptr->direct);
	for (int i = 0; i < ptr->cands.size(); ++i)
	{
		ss << "a=candidate:" << ptr->cands[i]->foundation
			<< " " << ptr->cands[i]->component
			<< " " << ptr->cands[i]->protocol
			<< " " << ptr->cands[i]->priority
			<< " " << ptr->cands[i]->address.HostAsURIString()
			<< " " << ptr->cands[i]->port
			<< " typ " << ptr->cands[i]->type
			<< "\r\n";
	}
}

void SessionDescription::add_media_direction(std::stringstream& ss, RtcDirection dir)
{
	if (dir == RtcDirection::RecvOnly)
	{
		ss << "a=recvonly\r\n";
	}
	else if (dir == RtcDirection::SendRecv)
	{
		ss << "a=sendrecv\r\n";
	}
	else if (dir == RtcDirection::SendOnly)
	{
		ss << "a=sendonly\r\n";
	}
	else
	{
		ss << "a=inactive\r\n";
	}
}

void SessionDescription::build(std::shared_ptr<SessionDescription> sdp)
{
	session.version = sdp->session.version;
	session.session_id = sdp->session.session_id;
	session.uni_addr = "0.0.0.0";
	session.username = "-";
}

void SessionDescription::create_answer(const RTCOfferAnswerOptions& options, const uvcore::IpAddress& addr)
{
	_addr = addr;
	RtcDirection direct = RtcDirection::Inactive;
	if ((options.recv_audio | options.recv_video) 
		&& (options.send_audio | options.send_video))
	{
		direct = RtcDirection::SendRecv;
	}
	else if ((options.send_audio | options.send_video)
		&& (options.recv_audio == false && options.recv_video == false))
	{
		direct = RtcDirection::SendOnly;
	}
	else if ((options.send_audio == false && options.send_video == false)
		&& (options.recv_audio || options.recv_video))
	{
		direct = RtcDirection::RecvOnly;
	}

	_fingerprint = rtc::SSLFingerprint::CreateFromCertificate(*Global::GetInstance()->get_dtls_certificate());
	if (!_fingerprint)
	{
		std::cerr << "_fingerprint generate error." << std::endl;
		return;
	}
	
	//audio
	auto audioptr = std::make_shared<AudioContentDesc>();
	audioptr->direct = direct;
	CodecInfo ci;
	ci.payload_type = 111;
	ci.desc = "opus/48000/2";
	ci.fbs.push_back(std::make_shared<FeedbackParameter>("transport-cc"));
	audioptr->rtpmaps.push_back(ci);

	FormatParameter fp;
	fp.codec_id = 111;
	fp.params["minptime"] = "10";
	fp.params["useinbandfec"] = "1";
	audioptr->fmtps.push_back(fp);


	//video
	auto vptr = std::make_shared<VideoContentDesc>();
	vptr->direct = direct;
	ci.payload_type = 108;
	ci.desc = "H264/90000";
	ci.fbs.clear();
	ci.fbs.push_back(std::make_shared<FeedbackParameter>("goog-remb"));
	ci.fbs.push_back(std::make_shared<FeedbackParameter>("nack"));
	ci.fbs.push_back(std::make_shared<FeedbackParameter>("nack", "pli"));
	vptr->rtpmaps.push_back(ci);
	ci.fbs.clear();
	ci.payload_type = 109;
	ci.desc = "rtx/90000";
	vptr->rtpmaps.push_back(ci);

	fp.codec_id = 108;
	fp.params.clear();
	fp.params["level-asymmetry-allowed"] = "1";
	fp.params["packetization-mode"] = "1";
	fp.params["profile-level-id"] = "42e01f";
	vptr->fmtps.push_back(fp);

	fp.codec_id = 109;
	fp.params.clear();
	fp.params["apt"] = "108";
	vptr->fmtps.push_back(fp);

	std::string ufrag = rtc::CreateRandomString(4);
	std::string passwd = rtc::CreateRandomString(24);
	IceParameter ipa;
	ipa.ufrag = ufrag;
	ipa.mode = "ice-lite";
	ipa.passwd = passwd;

	audioptr->ice = ipa;
	audioptr->connection_role = "passive";
	vptr->ice = ipa;
	vptr->connection_role = "passive";

	auto cand = std::make_shared<IceCandidate>(_addr.getIp(), _addr.getPort());
	cand->component = IceCandidateComponent::RTP;
	cand->username = ufrag;
	cand->password = passwd;
	cand->type = LOCAL_PORT_TYPE;
	cand->protocol = "udp";
	cand->foundation = cand->compute_foundation(cand->type, cand->protocol, "", cand->address);
	cand->priority = cand->get_priority(ICE_TYPE_PREFERENCE_HOST, 0, 0);

	audioptr->cands.push_back(cand);
	vptr->cands.push_back(cand);

	media_contents.push_back(audioptr);
	media_contents.push_back(vptr);
}

bool SessionDescription::parse_sdp(const std::vector<std::string>& vecs)
{
	bool flag = false;
	for (int i = 0; i < vecs.size(); ++i)
	{
		if (vecs[i].find("v=") != std::string::npos)
		{
			flag = parse_version(vecs[i]);
		}
		else if (vecs[i].find("o=") != std::string::npos)
		{
			flag = parse_origin(vecs[i]);
		}
		else if (vecs[i].find("m=") != std::string::npos)
		{
			flag = parse_media(vecs[i]);
		}
		else if (vecs[i].find("a=rtpmap") != std::string::npos)
		{
			flag = parse_rtpmap(vecs[i]);
		}

		if (!flag)
		{
			std::cerr << "parse sdp error: " << vecs[i] << std::endl;
			return false;
		}
	}
	return flag;
}

bool SessionDescription::parse_version(const std::string& sdp)
{
	std::vector<std::string> vecs;
	SUtil::split(sdp, "=", vecs);
	if (vecs.size() != 2)
	{
		return false;
	}
	session.version = vecs[1];
	return true;
}

bool SessionDescription::parse_origin(const std::string& sdp)
{
	std::vector<std::string> vecs;
	SUtil::split(sdp, "=", vecs);
	if (vecs.size() != 2)
	{
		return false;
	}
	std::vector<std::string> vecs2;
	SUtil::split(vecs[1], " ", vecs2);
	if (vecs2.size() != 6)
	{
		return false;
	}
	session.username = vecs2[0];
	session.session_id = vecs2[1];
	return true;
}

bool SessionDescription::parse_media(const std::string& sdp)
{
	std::vector<std::string> vecs;
	SUtil::split(sdp, "=", vecs);
	if (vecs.size() != 2)
	{
		return false;
	}

	std::vector<std::string> vecs2;
	SUtil::split(vecs[1], " ", vecs2);
	if (vecs2.size() < 4)
	{
		return false;
	}

	MediaCodecSupport sup;
	sup.port = std::atoi(vecs2[1].c_str());
	if (vecs2[2] == k_media_proto_dtls_savpf)
	{
		sup.dtls = true;
	}
	else if (vecs2[2] == k_media_proto_savpf)
	{
		sup.dtls = false;
	}
	else
	{
		return false;
	}
	for (int i = 3; i < vecs2.size(); ++i)
	{
		CodecInfo info;
		info.payload_type = std::atoi(vecs2[i].c_str());
		sup.codec_ids[std::atoi(vecs2[i].c_str())] = info;
	}
	//
	if (vecs2[0] == "video")
	{
		_video_sup = sup;
	}
	else if (vecs2[0] == "audio")
	{
		_audio_sup = sup;
	}
	else
	{
		return false;
	}
	return true;
}

bool SessionDescription::parse_rtpmap(const std::string& sdp)
{
	std::vector<std::string> vecs;
	SUtil::split(sdp, "=", vecs);
	if (vecs.size() != 2)
	{
		return false;
	}

	std::vector<std::string> vecs2;
	SUtil::split(vecs[1], " ", vecs2);
	if (vecs2.size() != 2)
	{
		return false;
	}
	size_t pos = vecs2[0].find(":");
	if (pos == std::string::npos)
	{
		return false;
	}
	std::string sid = vecs2[0].substr(pos + 1);
	int id = std::atoi(sid.c_str());
	auto it = _video_sup.codec_ids.find(id);
	if (it != _video_sup.codec_ids.end())
	{
		it->second.desc = vecs2[1];
		return true;
	}

	it = _audio_sup.codec_ids.find(id);
	if (it != _audio_sup.codec_ids.end())
	{
		it->second.desc = vecs2[1];
	}
	return true;
}

SessionDescription::~SessionDescription()
{
}