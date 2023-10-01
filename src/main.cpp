#include <crow/app.h>
#include <crow/common.h>
#include <crow/http_parser_merged.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <crow/middlewares/cookie_parser.h>
#include <exception>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "db.hpp"
#include "secret.hpp"


void               sendFile(crow::response& res, std::string filename, std::string content_type);
void               sendHtml(crow::response& res, std::string filename);
void               sendStyle(crow::response& res, std::string filename);
void               sendScript(crow::response& res, std::string filename);
void               sendImage(crow::response& res, std::string filename);
std::string        verifyAuthorization(const crow::request& req);
std::string        createNewUser(const crow::request& req);
std::string        getAuthUserId(crow::App<crow::CookieParser>& app, const crow::request& req);
std::string        getUserIdByApiKey(const std::string& apiKey);
bool               checkAuthorization(crow::App<crow::CookieParser>& app, const crow::request& req);

crow::json::wvalue sendErrorResponse(const std::string& errorMsg);
crow::json::wvalue sendResponse();
crow::json::wvalue sendResponse(const std::vector<std::vector<std::string>>& body);

crow::json::wvalue createNewProject(const std::string& name, const std::string& userId);
crow::json::wvalue createNewNote(const std::string& name, const std::string& projectName, const std::string& userId, const std::string& body);
crow::json::wvalue updateNote(const std::string& name, const std::string& projectName, const std::string& userId, const std::string& body);

crow::json::wvalue setApiKeyForUser(crow::App<crow::CookieParser>& app, const crow::request& req, const std::string& userId);

int main() {
    crow::App<crow::CookieParser> app;

    CROW_ROUTE(app, "/")([&](const crow::request& req, crow::response& res){
        if (!checkAuthorization(app, req)) return sendHtml(res, "index");
        res.redirect("/main");
        res.end();
    });

    CROW_ROUTE(app, "/css/<string>")([](const crow::request& req, crow::response& res, std::string filename){
        return sendStyle(res, filename);
    });

    CROW_ROUTE(app, "/js/<string>")([](const crow::request& req, crow::response& res, std::string filename){
        return sendScript(res, filename);
    });

    CROW_ROUTE(app, "/img/<string>")([](const crow::request& req, crow::response& res, std::string filename){
        return sendImage(res, filename);
    });

    CROW_ROUTE(app, "/main")([&](const crow::request& req, crow::response& res){
        if (checkAuthorization(app, req)) return sendHtml(res, "main");
        res.redirect("/");
        res.end();
    });

    CROW_ROUTE(app, "/auth").methods("POST"_method)([&](const crow::request& req) {
        std::string userId = verifyAuthorization(req);
        if (userId.empty()) return sendErrorResponse("your api-key is not valid");
        return setApiKeyForUser(app, req, userId);
    });


    CROW_ROUTE(app, "/reg").methods("POST"_method)([&](const crow::request& req) {
        std::string userId = createNewUser(req);
        if (userId.empty()) return sendErrorResponse("cannot create such user");
        return setApiKeyForUser(app, req, userId);
    });


    CROW_ROUTE(app, "/server").methods(crow::HTTPMethod::POST) 
        ([&](const crow::request& req) {
        std::string userId = getAuthUserId(app, req);
        if (userId.empty()) return sendErrorResponse("unauthorized user");
        if (req.body.size() == 0) {
            return sendResponse(
                    DB::Project::Select({DB::Project::name}).Where({
                        DB::Project::ownerId == DB::Int(userId),
                        })
                    );
        } else {
            auto rc = DB::Project::Select({DB::Project::id}).Where({
                        DB::Project::ownerId == DB::Int(userId),
                        DB::Project::name == DB::Str(req.body),
                    });
            if (rc.empty() || rc[0].empty()) return sendErrorResponse("such Project does not exist");

            auto noteList = DB::Note::Select({DB::Note::name, DB::Note::body, DB::Note::id}).Where({
                    DB::Note::projectId == DB::Int(rc[0][0]),
                    });
            std::vector<std::string> graphList;
            for (int i = 0; i < noteList.size(); ++i) {
                auto tmp = DB::Edge::Select({DB::Edge::dest}).Where({DB::Edge::start == DB::Int(noteList[i][2])});
                for (int j = 0; j < tmp.size(); ++j) {
                    graphList.push_back(fmt::format("{}:{}", noteList[i][2], tmp[j][0]));
                }
            }

            crow::json::wvalue x;
            x["status"] = "ok";
            x["body"] = noteList;
            x["graph"] = graphList;
            return x;
            /*return sendResponse(
                DB::Note::Select({DB::Note::name, DB::Note::body}).Where({
                    DB::Note::projectId == DB::Int(rc[0][0]),
                    })
                );
            */
        }
        return sendErrorResponse("invalid request arguments");
    });


    CROW_ROUTE(app, "/serv").methods(crow::HTTPMethod::POST)
        ([&](const crow::request& req) {
        std::string userId = getAuthUserId(app, req);
        if (userId.empty()) return sendErrorResponse("unauthorized user");
        crow::json::rvalue query = crow::json::load(req.body);
        if (!query.has("type")) return sendErrorResponse("bad reques");

        if (query["type"] == "Project") {
            return createNewProject(query["name"].s(), userId);
        }
        if (query["type"] == "Note") {
            if (!query.has("content")) {
                return createNewNote(query["data"].s(), query["parent"].s(), userId, "");
            } else {
                return updateNote(query["name"].s(), query["parent"].s(), userId, query["content"].s());
            }
        }
        return sendErrorResponse("unknown request Type");
    });

    CROW_ROUTE(app, "/logout")([&](const crow::request& req, crow::response& res){
        auto& ctx = app.get_context<crow::CookieParser>(req);
        ctx.set_cookie("auth-key", "");
        res.redirect("/");
        res.end();
    });

    app.port(8000).multithreaded().run();
    return 0;
}


void sendFile(crow::response& res, std::string filename, std::string content_type) {
    std::string ROOT_DIR = "../publics/";
    std::string filepath = ROOT_DIR.append(filename);
    try {
        std::ifstream input_content;
        input_content.open(filepath, std::ifstream::in);
        input_content.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        std::ostringstream string_stream;
        string_stream << input_content.rdbuf();
        input_content.close();

        res.set_header("Content-Type", content_type);
        res.write(string_stream.str());
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
        res.code = 404;
        res.write("content not found");
    }

    res.end();
}

void sendHtml(crow::response& res, std::string filename) {
    sendFile(res, filename.append(".html"), "text/html");
}

void sendStyle(crow::response& res, std::string filename) {
    std::string path = "css/" + filename;
    sendFile(res, path, "text/css");
}

void sendScript(crow::response& res, std::string filename) {
    std::string path = "js/" + filename;
    sendFile(res, path, "text/jscript");
}

void sendImage(crow::response& res, std::string filename) {
    std::string path = "img/" + filename;
    sendFile(res, path, "image/jpeg");
}

std::string verifyAuthorization(const crow::request& req) {
    std::string myauth = req.get_header_value("Authorization");
    /*Authorization: Basic VXNlcm5hbWU6UGFzc3dvcmQ= */
    std::string d_mycreds = myauth.substr(6);        // remove 'Basic '
    //std::string d_mycreds = crow::utility::base64decode(mycreds, mycreds.size());

    size_t found = d_mycreds.find(':');
    std::string username = d_mycreds.substr(0, found);
    std::string password = d_mycreds.substr(found+1);

    CodeStr(password);
    auto rc = DB::User::Select({DB::User::id, DB::User::password}).Where({DB::User::username == DB::Str(username)}); 
    if (!password.empty() && !rc.empty() && !rc[0].empty() && 
            password == rc[0][1]) /*db.SelectPassword(username))*/ {
        return rc[0][0];
    }

    return "";
}

std::string createNewUser(const crow::request& req) {
    std::string myreg = req.get_header_value("Registration");
    /*Authorization: Basic VXNlcm5hbWU6UGFzc3dvcmQ= */
    std::string d_mycreds = myreg.substr(6);        // remove 'Basic '
    //std::string d_mycreds = crow::utility::base64decode(mycreds, mycreds.size());

    size_t found = d_mycreds.find(':');
    std::string username = d_mycreds.substr(0, found);
    found = d_mycreds.find(':', found + 1);
    assert(found > username.size() && found - username.size() > 0);
    std::string email = d_mycreds.substr(username.size() + 1, found - (username.size() + 1));
    std::string password = d_mycreds.substr(found + 1);

    if (DB::User::Select({DB::User::id}).Where({DB::User::username == DB::Str(username)}).empty()) {
        CodeStr(password);

        std::string apiKey = GenApiKey(username);
        std::string date = GetDateAsStr();
        DB::User::Insert().Where({
                    DB::User::username == DB::Str(username),
                    DB::User::password == DB::Str(password),
                    DB::User::apiKey == DB::Str(apiKey),
                    DB::User::email == DB::Str(email),
                    DB::User::date == DB::Str(date),
                    });
            //if (db.CreateUser(username, password, apiKey, date, email)) {
        return DB::User::Select({DB::User::id}).Where({DB::User::username == DB::Str(username)})[0][0];
    }

    return "";
}


std::string getUserIdByApiKey(const std::string& apiKey) {
    auto rc = DB::User::Select({DB::User::id}).Where({DB::User::apiKey == DB::Str(apiKey)});
    if (rc.empty() || rc[0].empty()) return "";
    return rc[0][0];
}

std::string getAuthUserId(crow::App<crow::CookieParser>& app, const crow::request& req) {
    auto& ctx = app.get_context<crow::CookieParser>(req);
    std::string apiKey = ctx.get_cookie("auth-key");
    if (apiKey.empty()) return "";
    return getUserIdByApiKey(apiKey);
}

bool checkAuthorization(crow::App<crow::CookieParser>& app, const crow::request& req) {
    return !getAuthUserId(app, req).empty();
}

crow::json::wvalue sendErrorResponse(const std::string& errorMsg){
    crow::json::wvalue x;
    x["status"] = "fail";
    x["error"] = errorMsg;
    return x;
}

crow::json::wvalue sendResponse(const std::vector<std::vector<std::string>>& body){
    crow::json::wvalue x;
    x["status"] = "ok";
    x["body"] = body;
    return x;
}

crow::json::wvalue sendResponse(){
    crow::json::wvalue x;
    x["status"] = "ok";
    return x;
}

crow::json::wvalue createNewProject(const std::string& name, const std::string& userId) {
    if (name.size() > 100) return sendErrorResponse("the size of the name exceeded 100 characters");

    DB::Project::Insert().Where({
            DB::Project::name    == DB::Str(name),
            DB::Project::date    == DB::Str(GetDateAsStr()),
            DB::Project::ownerId == DB::Int(userId),
            });

    return sendResponse();

}

crow::json::wvalue createNewNote(const std::string& name, const std::string& projectName, const std::string& userId, const std::string& body) {
    auto rc = DB::Project::Select({DB::Project::id}).Where({
            DB::Project::name == DB::Str(projectName),
            DB::Project::ownerId == DB::Int(userId),
            });
    if (rc.empty() || rc[0].empty()) return sendErrorResponse("no such parent projest to create new note");

    DB::Note::Insert().Where({
            DB::Note::name == DB::Str(name),
            DB::Note::projectId == DB::Int(rc[0][0]),
            DB::Note::date == DB::Str(GetDateAsStr()),
            DB::Note::body == DB::Str(""),
            });
    return sendResponse();
}

crow::json::wvalue updateNote(const std::string& name, const std::string& projectName, const std::string& userId, const std::string& body) {
    auto rc = DB::Project::Select({DB::Project::id}).Where({
            DB::Project::name == DB::Str(projectName),
            DB::Project::ownerId == DB::Int(userId),
            });
    if (rc.empty() || rc[0].empty()) return sendErrorResponse("no such parent projest to create new note");

    DB::Note::Update({DB::Note::body == DB::Str(body)}).Where({
            DB::Note::name == DB::Str(name),
            DB::Note::projectId == DB::Int(rc[0][0]),
            });
    return sendResponse();
}

crow::json::wvalue setApiKeyForUser(crow::App<crow::CookieParser>& app, const crow::request& req, const std::string& userId) {
    auto& ctx = app.get_context<crow::CookieParser>(req);
    // Store cookies with set_cookie
    ctx.set_cookie("auth-key", DB::User::Select({DB::User::apiKey}).Where({DB::User::id == DB::Int(userId)})[0][0])
        // configure additional parameters
        .path("/")
        .max_age(60 * 60) // one hour
        .httponly()
        .same_site(crow::CookieParser::Cookie::SameSitePolicy::Lax);
    return sendResponse();
}
