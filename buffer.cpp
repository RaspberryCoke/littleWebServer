#include "buffer.h"
buffer::buffer() : _buff(1024), _first_pos{0}, _second_pos{0}
{
	_buff.resize(1024, 0);
}

bool buffer::extend()
{
	try
	{
		_buff.resize(_buff.capacity() * 2);
	}
	catch (...)
	{
		clog << "[err]:buffer:extend throw error" << endl;
		_buff.resize(0);
		return false;
	}
	return true;
}

void buffer::retrieve(int len)
{
	_first_pos += len;
}

void buffer::retrieve_until(const char *end)
{
	_first_pos += end - (&*_buff.begin() + _first_pos);
}

void buffer::retrieve_all()
{
	_first_pos = _second_pos = 0;
}

string buffer::retrieve_all_to_str()
{
	string str(get_f_ptr(), get_s_ptr());
	_first_pos = _second_pos = 0;
	return str;
}

void buffer::clear()
{
	clog << "[log]:buffer:clear()" << endl;
	_first_pos = _second_pos = 0;
	_buff.resize(0);
}

void buffer::shrink_to_fit()
{
	clog << "[log]:buffer:shrink_to_fit()" << endl;
	_buff.shrink_to_fit();
}

ostream &operator<<(ostream &os, buffer &buf)
{
	int size = buf.get_s_pos();
	for (int i = 0; i < size; i++)
	{
		os << buf._buff[i];
	}
	return os;
}

void buffer::Append(const std::string &str)
{
	Append(str.data(), str.length());
}

void buffer::Append(const void *data, size_t len)
{
	Append(static_cast<const char *>(data), len);
}

void buffer::Append(const char *str, size_t len)
{
	if (get_s_pos() - get_f_pos() < len)
	{
		extend();
	}
	std::copy(str, str + len, get_f_ptr());
	add_f_pos(len);
	add_s_pos(len);
}
void buffer::Append(const char *str, const int len)
{
	if (get_s_pos() - get_f_pos() < len)
	{
		extend();
	}
	std::copy(str, str + len, get_f_ptr());
	add_s_pos(len);
}

void buffer::Append(const buffer &buff)
{
	Append(buff.get_f_ptr(),get_f_pos());
}
