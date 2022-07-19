#include <juansfu/sdp/session_description.h>
#include <iostream>
#include <juansfu/utils/sutil.h>

const static std::string k_media_proto_dtls_savpf = "UDP/TLS/RTP/SAVPF";
const static std::string k_media_proto_savpf = "RTP/SAVPF";

SessionDescription::SessionDescription()
{
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

		if (!flag)
		{
			std::cerr << "parse sdp error: " << vecs[i] << std::endl;
			return false;
		}
	}
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

SessionDescription::~SessionDescription()
{
}