#include "secret.hpp"
#include <iomanip>
#include <ctime>

std::string CreateApiKey(const std::string& username, const std::string& data) {
    std::string res = username + data;
    CodeStr(res);
    return res;
}

void CodeStr(std::string& s) {
    /*
    for (char& c : s) {
        c = (char) (((int)c + w) % 128);
    }
    */
}

void DecodeStr(std::string& s) {
    /*
    for (char& c : s) {
        c = (char) (((int)c + w) % 128);
    }
    */
}

std::string GenApiKey(std::string username) {
    return CreateApiKey(username, GetDateAsStr());
}

std::string GetDateAsStr() {
    auto t = std::time(nullptr);
    std::string datetime(100,0);
    datetime.resize(std::strftime(&datetime[0], datetime.size(), "%d-%m-%Y", std::localtime(&t)));
    return datetime;
}

