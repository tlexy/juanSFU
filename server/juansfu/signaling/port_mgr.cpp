#include <juansfu/signaling/port_mgr.h>
#include <iostream>

UdpPortManager::UdpPortManager()
{
}

void UdpPortManager::init(int start, int end, int step)
{
	int len = (end - start) / step;
	if (len > 0)
	{
		_port_array = new unsigned[len];
		_port_array_rec = new unsigned[len];
		for (int i = 0; i < len; ++i)
		{
			_port_array[i] = start + i * step;
			_port_array_rec[i] = 0;
		}
	}
	else
	{
		std::cerr << "udp port manager init failed, start: " << start << ", end: " << std::endl;
	}
	_max = len;
	_curr = 0;
}

unsigned UdpPortManager::allocate_port()
{
	if (_curr >= _max)
	{
		if (_recycle > 0)
		{
			for (int i = 0; i < _recycle; ++i)
			{
				_port_array[--_curr] = _port_array_rec[i];
			}
			_recycle = 0;
		}
		else
		{
			return 0;
		}
	}
	unsigned port = _port_array[_curr++];
	return port;
}

void UdpPortManager::recycle_port(unsigned port)
{
	if (_recycle >= _max)
	{
		std::cerr << "udp port manager recycle error, max: " << _max << ", curr: " << _curr << ", recycle: " << _recycle << std::endl;
		return;
	}
	_port_array_rec[_recycle++] = port;
}

UdpPortManager::~UdpPortManager()
{
}