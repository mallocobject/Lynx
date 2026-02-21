#include <cppconn/exception.h>
#include <lynx/lynx.hpp>

using namespace lynx;

int main()
{
	try
	{
		Session sess(InetAddr(3306, true), "liudan", "liudan");
		Schema db = sess.schema("mysql");

		db.createTable("Passerby", true)
			.addColumn("uid", Type::INT)
			.addColumn("username", Type::STRING, 50)
			.addColumn("password", Type::STRING, 255)
			.addColumn("email", Type::STRING, 100)
			.primaryKey("uid")
			.execute();

		LOG_INFO << "table ready";

		Table table = db.table("Passerby");

		table.insert("uid", "username", "password", "email")
			.values(1, "danking", "admin1", "danking@mail.ustc.edu.cn")
			.values(2, "liudan", "321654", "2328211960@qq.com")
			.execute();

		table.update().set("password", "123456").where("uid = 2").execute();

		table.select("email").where("uid > 0").execute().printAll();

		table.remove().where("uid = 2").execute();
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "MySQL Error: " << e.what() << std::endl;
	}

	return 0;
}
