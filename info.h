#ifndef __INFO_H__
#define __INFO_H__

#include <cassert>

#include <mutex>

#include "util.h"

class InfoManager {
public:
    InfoManager(const std::string &host, uint16_t port, const std::string &user, 
                const std::string &password, const std::string &database, const std::string &csname)
        : mysql_(nullptr) {
        mysql_ = UtilMysql::Create(host, port, user, password, database, csname);
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