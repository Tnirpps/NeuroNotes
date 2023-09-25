#include <crow/app.h>
#include <crow/common.h>
#include <crow/http_parser_merged.h>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/json.h>
#include <crow/middlewares/cookie_parser.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "db.hpp"
#include "secret.hpp"



void sendFile(crow::response& res, std::string filename, std::string content_type);
void sendHtml(crow::response& res, std::string filename);
void sendStyle(crow::response& res, std::string filename);
void sendScript(crow::response& res, std::string filename);
void sendImage(crow::response& res, std::string filename);
std::string verifyAuthorization(const crow::request& req);
std::string createNewUser(const crow::request& req);


int main() {
    crow::App<crow::CookieParser> app;

    CROW_ROUTE(app, "/")([&](const crow::request& req, crow::response& res){
        auto& ctx = app.get_context<crow::CookieParser>(req);
        // Read cookies with get_cookie
        std::string apiKey = ctx.get_cookie("auth-key");
        if (DB::User::Select({DB::User::id}).Where({DB::User::apiKey == DB::Str(apiKey)}).empty()) {
//        if (db.SelectUserByApiKey(apiKey).empty()) {
            return sendHtml(res, "index");
        }
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
        auto& ctx = app.get_context<crow::CookieParser>(req);
        // Read cookies with get_cookie
        std::string apiKey = ctx.get_cookie("auth-key");
        if (!DB::User::Select({DB::User::id}).Where({DB::User::apiKey == DB::Str(apiKey)}).empty()) {
//        if (!db.SelectUserByApiKey(apiKey).empty()) {
            return sendHtml(res, "main");
        }
        res.redirect("/");
        res.end();
    });

    CROW_ROUTE(app, "/auth").methods("POST"_method)([&](const crow::request& req) {
        std::string username = verifyAuthorization(req);
        if (username.empty()) return "";

        auto& ctx = app.get_context<crow::CookieParser>(req);
        // Store cookies with set_cookie
        ctx.set_cookie("auth-key", DB::User::Select({DB::User::apiKey}).Where({DB::User::username == DB::Str(username)})[0][0])
                                             // configure additional parameters
        .path("/")
        .max_age(60 * 60) // one hour
        .httponly()
        .same_site(crow::CookieParser::Cookie::SameSitePolicy::Lax);
        return "ok!";
    });


    CROW_ROUTE(app, "/reg").methods("POST"_method)([&](const crow::request& req) {
        std::string username = createNewUser(req);
        if (username.empty()) return "";
        auto& ctx = app.get_context<crow::CookieParser>(req);
        // Store cookies with set_cookie
        ctx.set_cookie("auth-key", DB::User::Select({DB::User::apiKey}).Where({DB::User::username == DB::Str(username)})[0][0])
        //ctx.set_cookie("auth-key", db.SelectApiKey(username))
          // configure additional parameters
          .path("/")
          .max_age(60 * 60) // one hour
          .httponly()
          .same_site(crow::CookieParser::Cookie::SameSitePolicy::Lax);
        return "ok!";
    });

    CROW_ROUTE(app, "/server").methods(crow::HTTPMethod::POST) 
        ([&](const crow::request& req) {
        crow::json::wvalue x;
        auto& ctx = app.get_context<crow::CookieParser>(req);
        std::string apiKey = ctx.get_cookie("auth-key");
        auto rc = DB::User::Select({DB::User::id}).Where({DB::User::apiKey == DB::Str(apiKey)});
        if (rc.empty() || rc[0].empty()) {
            x["status"] = "fail";
            x["error"] = "unauthorized user";
            return x;
        }
        if (req.body.size() == 0) {
            x["projects"] = DB::Project::Select({DB::Project::name}).Where({
                    DB::Project::ownerId == DB::Int(rc[0][0]),
                    });
            x["status"] = "ok";
        } else {
            auto rc = DB::Project::Select({DB::Project::id}).Where({
                        DB::Project::name == DB::Str(req.body),
                    });
            if (rc.empty() || rc[0].empty()) {
                x["status"] = "fail";
            } else {
                std::cout << "FIND::::\t" << rc[0][0] << "\n";
                x["notes"] = DB::Note::Select({DB::Note::name}).Where({
                        DB::Note::projectId == DB::Int(rc[0][0]),
                        });
                x["status"] = "ok";
            }
        }
        return x;

    });


    CROW_ROUTE(app, "/serv").methods(crow::HTTPMethod::POST)
        ([&](const crow::request& req) {
        crow::json::wvalue x;
        auto& ctx = app.get_context<crow::CookieParser>(req);
        std::string apiKey = ctx.get_cookie("auth-key");
        auto rc = DB::User::Select({DB::User::id}).Where({DB::User::apiKey == DB::Str(apiKey)});
        if (rc.empty() || rc[0].empty()) {
            x["status"] = "fail";
            x["error"] = "unauthorized user";
            return x;
        }
        crow::json::rvalue query = crow::json::load(req.body);
        if (query["type"] == "Project") {
            if (req.body.size() > 100) {
                x["status"] = "fail";
                x["error"] = "the size of the name exceeded 100 characters";
                return x;
            }

            DB::Project::Insert().Where({
                    DB::Project::name    == DB::Str(query["data"].s()), /*TODO: escape*/
                    DB::Project::date    == DB::Str(GetDateAsStr()),
                    DB::Project::ownerId == DB::Int(rc[0][0]),
                    });

            x["status"] = "ok";
        } else if (query["type"] == "Note") {
            auto rc = DB::Project::Select({DB::Project::id}).Where({DB::Project::name == DB::Str(query["parent"].s())});
            if (rc.empty() || rc[0].empty()) {
                x["status"] = "fail";
                x["error"] = "no such parent projest to create new note";
                return x;
            }

            DB::Note::Insert().Where({
                    DB::Note::name == DB::Str(query["data"].s()),
                    DB::Note::projectId == DB::Int(rc[0][0]),
                    DB::Note::date == DB::Str(GetDateAsStr()),
                    DB::Note::body == DB::Str(""),
                    });
            x["status"] = "ok";
        } else {
            x["status"] = "fail";
            std:: cout << "\n\n\tERROR\n\n";
        }
        return x;
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
    auto rc = DB::User::Select({DB::User::password}).Where({DB::User::username == DB::Str(username)}); 
    if (!password.empty() && !rc.empty() && !rc[0].empty() && 
            password == rc[0][0]) /*db.SelectPassword(username))*/ {
        return username;
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

    CodeStr(password);

    std::string apiKey = GenApiKey(username);
    std::string date = GetDateAsStr();
    if (DB::User::Insert().Where({
                DB::User::username == DB::Str(username),
                DB::User::password == DB::Str(password),
                DB::User::apiKey == DB::Str(apiKey),
                DB::User::email == DB::Str(email),
                DB::User::date == DB::Str(date),
                })) {
    //if (db.CreateUser(username, password, apiKey, date, email)) {
        return username;
    }
    return "";
}










