#ifndef JUAN_SRTP_SUBSCRIBER_H
#define JUAN_SRTP_SUBSCRIBER_H

#include <stdint.h>

class SrtpSubscriber
{
public:
	SrtpSubscriber();
	~SrtpSubscriber();

	//输入是解码后的rtp/rtcp数据
	virtual void on_rtp_packet(void* data, int len) = 0;
	virtual void on_rtcp_packet(void* data, int len) = 0;

private:

};

#endif