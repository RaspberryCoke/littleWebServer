#include "sqlpool.h"
sqlpool::sqlpool(int sqlPort, string sqlName, string sqlPass, string databaseName)
{
	clog << "[log]:sqlpool:sqlpool()" << endl;
	MYSQL* sql;
	sql = mysql_init(nullptr);
	if (sql == nullptr)
	{
		clog << "[err]:sqlpool:sqlpool() mysql_init error" << endl;
		assert(sql != nullptr);
	}
	string host = "localhost";
	sql = mysql_real_connect(sql, host.c_str(), sqlName.c_str(), sqlPass.c_str(), databaseName.c_str(), 3036, nullptr, 0);
	if (sql == nullptr)
	{
		clog << "[err]:sqlpool:sqlpool() mysql_real_connect error" << endl;
		assert(sql != nullptr);
	}
	_sql = sql;
}
sqlpool::~sqlpool()
{
	mysql_close(_sql);
	mysql_library_end();
}