#ifndef DATABASE_LIB
#define DATABASE_LIB

#include <fmt/core.h>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include <fmt/format.h>

namespace DB {
    const std::string user     = "tnirpps";
    const std::string password = "1234";
    const std::string host     = "localhost";
    const std::string port     = "5433";
    const std::string dbName   = "neuro-notes";

    class Condition;

    class Column {
        public:
            std::string name;
            Column(const std::string& c) : name(c) {};
    };

    class Condition {
        private:
            std::string key;
            std::string value;
            Condition(const std::string& key, const std::string& value): key(key), value(value) {}
        public:
            std::string GetKey() const;
            std::string GetValue() const;
            std::string GetEqForm() const;

            friend class IntColumn;
            friend class StrColumn;
    };

    class Int : public Column {
        public:
            Int(const std::string& c) : Column(c) {}
            Int(int c) : Column(std::to_string(c)) {}
    };

    class Str : public Column {
        public:
            Str(const std::string& c) : Column(c) {}
            Str(Column c) : Column(c) {}
    };

    class IntColumn : public Column {
        public:
            Condition operator == (const DB::Int& other) const {
                return Condition(name, other.name);
            }
            IntColumn(const std::string& s) : Column(s) {};
    };

    class StrColumn : public Column {
        public:
            Condition operator == (const DB::Str& other) const {
                return Condition(name, fmt::format("'{}'", other.name));
            }
            // TODO: == для DB:Int просто кастует его к Str, вметсо ошибки
            StrColumn(const std::string& s) : Column(s) {};
    };


    class Connection {
        public:
            pqxx::connection Connect();
    };

    class SelectQuery : private Connection {
        private:
            std::string tableName;
            std::vector<Column> res;

        public:
            std::string CreateSqlQueryString(const std::vector<Condition>& v);
            SelectQuery(const std::string& s, const std::vector<Column>& v): tableName(s), res(v) {}

            std::vector<std::vector<std::string>> Where(const std::vector<Condition>& v);
    };

    class InsertQuery : private Connection {
        private:
            std::string tableName;
        public:
            std::string CreateSqlQueryString(const std::vector<Condition>& v);
            InsertQuery(const std::string& s): tableName(s) {}

            bool Where(const std::vector<Condition>& v);

    };

    class UpdateQuery : private Connection {
        private:
            std::string tableName;
            std::vector<Condition> res;
            //pqxx::connection Connect();

        public:
            std::string CreateSqlQueryString(const std::vector<Condition>& v);
            UpdateQuery(const std::string& s, const std::vector<Condition>& v): tableName(s), res(v) {}

            bool Where(const std::vector<Condition>& v);
    };

    class RemoveQuery : private Connection {
        private:
            std::string tableName;
        public:
            std::string CreateSqlQueryString(const std::vector<Condition>& v);
            RemoveQuery(const std::string& s): tableName(s){}

            bool Where(const std::vector<Condition>& v);
    };

    class User {
        
        static const std::string tableName;
        
        public:

            /* list of columns names in table `users` */
            static const IntColumn id;
            static const StrColumn username;
            static const StrColumn password;
            static const StrColumn apiKey;
            static const StrColumn date;
            static const StrColumn email;

            static SelectQuery Select(const std::vector<Column>& queryCols);
            static InsertQuery Insert();
            static RemoveQuery Remove();
    };

    class Project {
        
        static const std::string tableName;
        
        public:

            /* list of columns names in table `projects` */
            static const IntColumn id;
            static const StrColumn name;
            static const StrColumn date;
            static const IntColumn ownerId;

            static SelectQuery Select(const std::vector<Column>& queryCols);
            static InsertQuery Insert();
            static RemoveQuery Remove();
    };

    class Note {
        
        static const std::string tableName;
        
        public:

            /* list of columns names in table `notes` */
            static const IntColumn id;
            static const StrColumn name;
            static const StrColumn body;
            static const StrColumn date;
            static const IntColumn projectId;

            static const IntColumn posX;
            static const IntColumn posY;

            static SelectQuery Select(const std::vector<Column>& queryCols);
            static UpdateQuery Update(const std::vector<Condition>& queryCols);
            static InsertQuery Insert();
            static RemoveQuery Remove();
    };

    class Edge {
        
        static const std::string tableName;
        
        public:

            /* list of columns names in table `notes` */
            static const IntColumn id;
            static const IntColumn start;
            static const IntColumn dest;

            static SelectQuery Select(const std::vector<Column>& queryCols);
            static InsertQuery Insert();
            static RemoveQuery Remove();
    };
};

#endif /* ifndef DATABASE_LIB */

