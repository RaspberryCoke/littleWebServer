#pragma once
#include<iostream>
#include<string>
#include"assert.h"
#include"mysql/mysql.h"
using namespace std;
class sqlpool
{
public:
	sqlpool(int sqlPort, string sqlName, string sqlPass, string databaseName);
	~sqlpool();
private:
	MYSQL* _sql;
};

