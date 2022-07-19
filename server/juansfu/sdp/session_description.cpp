#include <juansfu/sdp/session_description.h>
#include <iostream>
#include <juansfu/utils/sutil.h>

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

	ss << "a=group:BUNDLE audio video" << "\r\n";

	for (int i = 0; i < media_contents.size(); ++i)
	{
		add_media_content(ss, media_contents[i]);
	}

	return ss.str();
}

void SessionDescription::add_media_content(std::stringstream& ss, std::shared_ptr<MediaContent> ptr)
{
	ss << "m=" << ptr->mid() << " 9 ";
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

}

void SessionDescription::build(std::shared_ptr<SessionDescription> sdp)
{
	session.version = sdp->session.version;
	session.session_id = sdp->session.session_id;
	session.uni_addr = "0.0.0.0";
	session.username = "-";
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