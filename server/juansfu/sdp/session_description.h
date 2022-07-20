﻿#ifndef JUAN_SESSION_DESCRIPTION_H
#define JUAN_SESSION_DESCRIPTION_H

#include <string>
#include <stdint.h>
#include <vector>
#include <memory>
#include <sstream>
#include <unordered_map>

class SessionSdp
{
public:
	//v=
	std::string version;
	//o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
	std::string username = "";
	std::string session_id;
	std::string session_version = "2";
	std::string nett_type = "IN";
	std::string addr_type = "IP4";
	std::string uni_addr;
	//s=
	std::string session_name = "-";
	//t=
	std::string ts = "0 0";
	std::string attr = "a=msid-semantic: WMS";
};

enum class MediaType {
	MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_VIDEO
};

class CodecInfo
{
public:
	int payload_type;
	std::string desc;
};

class MediaCodecSupport
{
public:
	std::string name;
	int port;
	bool dtls;
	std::unordered_map<int, CodecInfo> codec_ids;
};

class MediaContent {
public:
	virtual ~MediaContent() {}
	virtual MediaType type() = 0;
	virtual std::string mid() = 0;

public:
	std::string connection_info = "IN IP4 0.0.0.0";
	std::string fix_attr1 = "rtcp:9 IN IP4 0.0.0.0";
	std::vector<CodecInfo> rtpmaps;
	bool use_dtls = true;
};

class AudioContentDesc : public MediaContent {
public:
	MediaType type() override { return MediaType::MEDIA_TYPE_AUDIO; }
	std::string mid() override { return "audio"; }
};

class VideoContentDesc : public MediaContent {
public:
	MediaType type() override { return MediaType::MEDIA_TYPE_VIDEO; }
	std::string mid() override { return "video"; }
};

class SessionDescription
{
public:
	SessionDescription();
	~SessionDescription();

	bool parse_sdp(const std::vector<std::string>& vecs);

	void build(std::shared_ptr<SessionDescription>);
	std::string to_string();

public:
	SessionSdp session;
	std::vector<std::shared_ptr<MediaContent>> media_contents;

private:
	MediaCodecSupport _video_sup;
	MediaCodecSupport _audio_sup;

private:
	void add_media_content(std::stringstream&, std::shared_ptr<MediaContent>);

private:
	bool parse_version(const std::string&);
	bool parse_origin(const std::string&);
	bool parse_media(const std::string&);
	bool parse_rtpmap(const std::string&);
};

#endif