#ifndef JUAN_PORT_ALLOCATOR_H
#define JUAN_PORT_ALLOCATOR_H

#include <juansfu/utils/singleton.h>
#include <string>

class UdpPortManager : public Singleton<UdpPortManager>
{
public:
	UdpPortManager();
	/// <summary>
	/// 
	/// </summary>
	/// <param name="start">起始端口</param>
	/// <param name="end">终止端口</param>
	/// <param name="step">每个客户端占用端口个数</param>
	void init(int start, int end, int step);

	unsigned allocate_port();
	void recycle_port(unsigned port);

	~UdpPortManager();

public:
	std::string ipstr;

private:
	unsigned* _port_array{NULL};
	unsigned* _port_array_rec{ NULL };
	int _max{0};
	int _curr{0};
	int _recycle{0};

};

#endif