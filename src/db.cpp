#include "db.hpp"
#include <iostream> // TODO: remove

std::string DB::Condition::GetKey() const {
    return key;
}

std::string DB::Condition::GetValue() const {
    return value;
}

std::string DB::Condition::GetEqForm() const {
    return fmt::format("{} = {}", key, value);
}


pqxx::connection DB::SelectQuery::Connect() {
    return pqxx::connection(fmt::format(
                "user={} password={} host={} port={} dbname={}",
                user, password, host, port, dbName
                ));
}

std::string DB::SelectQuery::CreateSqlQueryString(const std::vector<Condition>& c) {
    std::string query = "select ";
    for (size_t i = 0; i + 1 < res.size(); ++i) {
        query += (res[i].name + ", ");
    }
    if (res.empty()) {
        query += "* ";
    } else {
        query += (res.back().name + " ");
    }
    query += "from " + tableName;
    if (!c.empty()) {
        query += " where ";

        for (size_t i = 0; i + 1 < c.size(); ++i) {
            query += (c[i].GetEqForm() + " AND ");
        }
        query += c.back().GetEqForm();
    }
    std::cout << "CREATED QUERY:: " << query << "\n";
    return query;
}    


std::vector<std::vector<std::string>> DB::SelectQuery::Where(const std::vector<Condition>& v) {
    auto c = Connect();
    pqxx::work tx{c};
    auto rc = tx.exec(CreateSqlQueryString(v));
    std::vector<std::vector<std::string>> res;
    for (const auto& r: rc) {
        res.push_back({});
        for (const auto& s: r) {
            res.back().push_back(s.as<std::string>());
        }
    }
    tx.commit();
    c.close();
    return res;
}

DB::SelectQuery DB::User::Select(const std::vector<Column>& queryCols) {
    return SelectQuery(tableName, queryCols);
}




pqxx::connection DB::InsertQuery::Connect() {
    return pqxx::connection(fmt::format(
                "user={} password={} host={} port={} dbname={}",
                user, password, host, port, dbName
                ));
}

std::string DB::InsertQuery::CreateSqlQueryString(const std::vector<Condition>& c) {
    if (c.empty()) return "error";
    std::string query = fmt::format("insert into {} (", tableName);
    for (size_t i = 0; i + 1 < c.size(); ++i) {
        query += (c[i].GetKey() + ", ");
    }
    query += (c.back().GetKey() + ") values (");

    for (size_t i = 0; i + 1 < c.size(); ++i) {
        query += (c[i].GetValue() + ",");
    }
    query += (c.back().GetValue() + ")");

    std::cout << "CREATED QUERY:: " << query << "\n";
    return query;
}    

bool DB::InsertQuery::Where(const std::vector<Condition>& v) {
    auto c = Connect();
    pqxx::work tx{c};
    auto res = tx.exec(CreateSqlQueryString(v));
    tx.commit();
    c.close();
    return true;
}



std::string DB::UpdateQuery::CreateSqlQueryString(const std::vector<Condition>& c) {
    std::string query = fmt::format("update {} set ", tableName);
    if (res.empty()) return "ERROR";
    for (size_t i = 0; i + 1 < res.size(); ++i) {
        query += res[i].GetEqForm() + ", ";
    }
    query += res.back().GetEqForm();
    if (!c.empty()) {
        query += " where ";

        for (size_t i = 0; i + 1 < c.size(); ++i) {
            query += (c[i].GetEqForm() + " AND ");
        }
        query += c.back().GetEqForm();
    }
    std::cout << "CREATED QUERY:: " << query << "\n";
    return query;
}    

bool DB::UpdateQuery::Where(const std::vector<Condition>& v) {
    auto c = Connect();
    pqxx::work tx{c};
    auto res = tx.exec(CreateSqlQueryString(v));
    tx.commit();
    c.close();
    return true;
}

pqxx::connection DB::UpdateQuery::Connect() {
    return pqxx::connection(fmt::format(
                "user={} password={} host={} port={} dbname={}",
                user, password, host, port, dbName
                ));
}

DB::InsertQuery DB::User::Insert() {
    return InsertQuery(tableName);
}





const std::string DB::User::tableName = "users";

const DB::IntColumn DB::User::id("id");
const DB::StrColumn DB::User::username("username");
const DB::StrColumn DB::User::password("password");
const DB::StrColumn DB::User::apiKey("api_key");
const DB::StrColumn DB::User::date("created_on");
const DB::StrColumn DB::User::email("email");



DB::InsertQuery DB::Project::Insert() {
    return InsertQuery(tableName);
}

DB::SelectQuery DB::Project::Select(const std::vector<Column>& queryCols) {
    return SelectQuery(tableName, queryCols);
}

const std::string DB::Project::tableName = "projects";

const DB::IntColumn DB::Project::id("id");
const DB::StrColumn DB::Project::name("name");
const DB::StrColumn DB::Project::date("created_on");
const DB::IntColumn DB::Project::ownerId("owner");





DB::InsertQuery DB::Note::Insert() {
    return InsertQuery(tableName);
}

DB::SelectQuery DB::Note::Select(const std::vector<Column>& queryCols) {
    return SelectQuery(tableName, queryCols);
}

DB::UpdateQuery DB::Note::Update(const std::vector<Condition>& queryCols) {
    return UpdateQuery(tableName, queryCols);
}

const std::string DB::Note::tableName = "notes";

const DB::IntColumn DB::Note::id("id");
const DB::StrColumn DB::Note::name("name");
const DB::StrColumn DB::Note::body("body");
const DB::StrColumn DB::Note::date("created_on");
const DB::IntColumn DB::Note::projectId("project");



DB::InsertQuery DB::Edge::Insert() {
    return InsertQuery(tableName);
}

DB::SelectQuery DB::Edge::Select(const std::vector<Column>& queryCols) {
    return SelectQuery(tableName, queryCols);
}


const std::string DB::Edge::tableName = "edges";

const DB::IntColumn DB::Edge::id("id");
const DB::IntColumn DB::Edge::start("start");
const DB::IntColumn DB::Edge::dest("dest");

