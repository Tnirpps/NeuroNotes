#include "database.hpp"

std::string ColumnToString(Column c) {
    switch (c) {
        case Column::username:
            return "username";
        case Column::password:
            return "password";
        case Column::apiKey:
            return "api_key";
        case Column::date:
            return "created_on";
        case Column::email:
            return "email";
        default:
            assert("Unreacheble" && 0);
    }
}

Database::Database(std::string user, std::string password, std::string host, std::string port, std::string dbName):
    user(user), password(password), host(host), port(port), dbName(dbName){}

bool Database::UserExist(const std::string& username) {
    pqxx::connection c = Connect();
    pqxx::work tx{c};
    auto x = tx.query<std::string>(fmt::format(
                "select username from users where username='{}'", tx.esc(username)
                ));
    tx.commit();
    c.close();
    return x.begin() != x.end();
}

bool Database::CreateUser(const std::string& username, const std::string& password, const std::string& apiKey, const std::string& date, const std::string& email) {
    if (UserExist(username)) return false;
    pqxx::connection c = Connect();
    pqxx::work tx{c};

    tx.exec(fmt::format(
                "insert into users (username, password, api_key, created_on, email)\
                values ('{}', '{}', '{}', '{}', '{}')",
                tx.esc(username), tx.esc(password), tx.esc(apiKey), tx.esc(date), tx.esc(email)
                ));

    tx.commit();
    c.close();
    return true;
}


std::string Database::SelectPassword(const std::string& username) {
    auto res = SelectByColumn(Column::password, Column::username, username);
    if (res.begin() == res.end()) return "";
    return res.begin()[0].as<std::string>();
}

std::string Database::SelectApiKey(const std::string& username) {
    auto res = SelectByColumn(Column::apiKey, Column::username, username);
    if (res.begin() == res.end()) return "";
    return res.begin()[0].as<std::string>();
}

std::string Database::SelectUserByApiKey(const std::string& apiKey) {
    auto res = SelectByColumn(Column::username, Column::apiKey, apiKey);
    if (res.begin() == res.end()) return "";
    return res.begin()[0].as<std::string>();
}

pqxx::result Database::SelectByColumn(Column column, Column by, std::string value) {
    pqxx::connection c = Connect();
    pqxx::work tx{c};
    std::string columnName = ColumnToString(column);
    std::string byName = ColumnToString(by);
    auto res = tx.exec(fmt::format(
                "select {} from users where {}='{}'", columnName, byName, tx.esc(value)
                ));
    tx.commit();
    c.close();
    return res;
}

pqxx::connection Database::Connect() {
    return pqxx::connection(fmt::format(
                "user={} password={} host={} port={} dbname={}",
                user, password, host, port, dbName
                ));
}

