#pragma once
#include<iostream>
#include<vector>
#include<string>
using namespace std;
class buffer
{
public:
	buffer();
	
	void add_s_pos(int len){_second_pos+=len;}
	void add_f_pos(int len){_first_pos+=len;}
	const int get_f_pos() const{ return _first_pos; }
	char* get_begin_ptr(){return &*_buff.begin();}
	int get_f_pos() { return _first_pos; }
	const int get_s_pos() const{ return _second_pos; }
	int get_s_pos() { return _second_pos; }
	const char* get_f_ptr() const{ return &*(_buff.begin() + _first_pos); }
	char* get_f_ptr() { return &*(_buff.begin() + _first_pos); }
	const char* get_s_ptr() const{ return &*(_buff.begin() + _second_pos); }
	char* get_s_ptr() { return &*(_buff.begin() + _second_pos); }

	const int size() { return _buff.capacity() - _second_pos; }//?

	bool extend();
	void clear();
	void shrink_to_fit();
	

	void retrieve(int);
	void retrieve_until(const char*);
	void retrieve_all();
	string retrieve_all_to_str();

	void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const buffer& buff);
	void Append(const char* str, const int len);

	friend ostream& operator<<(ostream& os,buffer& buf);
public:
	vector<char> _buff;
	int _first_pos;
	int _second_pos;
};

