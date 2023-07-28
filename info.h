#ifndef __INFO_H__
#define __INFO_H__

#include <cassert>

#include <mutex>

#include "util.h"

#define HOST "127.0.0.1"
#define PORT 3306
#define USER "root"
#define PASSWORD "Wang2923944542"
#define DATABASE "gobang"
#define CSNAME "utf8"

class InfoManager {
public:
    InfoManager() : mysql_(nullptr) {
        mysql_ = UtilMysql::Create(HOST, PORT, USER, PASSWORD, DATABASE, CSNAME);
        assert(nullptr != mysql_);
    }
    ~InfoManager() {
        UtilMysql::Destroy(mysql_);
        mysql_ = nullptr;
    }
    bool Register(const Json::Value &root);
    bool Login(Json::Value *root);
    bool GetInfoByUsername(const std::string &username, Json::Value *root);
    bool GetInfoByUid(uint64_t uid, Json::Value *root);
    bool Winner(uint64_t uid);
    bool Loser(uint64_t uid);
private:
    MYSQL *mysql_;
    std::mutex mutex_;
};

#endif //__INFO_H__