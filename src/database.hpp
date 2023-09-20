#include <string>
#include <fmt/core.h>
#include <pqxx/pqxx>
#include <vector>

#ifndef DATABASE_LIB
#define DATABASE_LIB

enum Column {
    username,
    password,
    apiKey,
    date,
    email
};

std::string ColumnToString(Column c);


namespace DB {
    namespace {
        std::string user = "aboba";
        std::string password;
        std::string host;
        std::string port;
        std::string dbName;
    };
    class Column {
        public:
            std::string name;
    };

    class IntColumn : public Column {
        public:
            std::string operator == (const IntColumn &other) {
                return name + " = " + other.name;
            }
    };

    class DBInt : public Column {
    };

    class SelectQuery {
        std::vector<Column> v;
        pqxx::connection Connect();

        pqxx::result Where(std::string s) {
            auto c = Connect();
            pqxx::work tx{c};
            return tx.exec("select ...");
        }

    };


    class User {
        IntColumn id;
        void select() {
            return;
        }
    };



    // select * from users where
};
//DB::User::select(user::id).where(User::id == DBInt(1)); // ->


/*
class User {
    id = "id";
    apikey = "GGG";
    select();
};
*/

class Database {






    public:
        Database(std::string user, std::string password, std::string host, std::string port, std::string dbName);
        bool UserExist(const std::string& username);

        bool CreateUser(const std::string& username, const std::string& password, const std::string& apiKey, const std::string& date, const std::string& email);

        std::string SelectPassword(const std::string& username);
        std::string SelectApiKey(const std::string& username);
        std::string SelectUserByApiKey(const std::string& apiKey);
    private:
        std::string user;
        std::string password;
        std::string host;
        std::string port;
        std::string dbName;

        pqxx::result SelectByColumn(Column column, Column by, std::string value);
        pqxx::connection Connect();
};


#endif /* ifndef DATABASE_LIB */

