#include "info.h"

#define REGISTER_FORMAT "insert user values (null, '%s', password('%s'), 0, 0, 1000)"
#define LOGIN_FORMAT "select uid, total_games, total_wins, score from user where username='%s' and password=password('%s');"
#define GETINFOBYUSERNAME_FORMAT "select uid, total_games, total_wins, score from user where username='%s';"
#define GETINFOBYUID_FORMAT "select username, total_games, total_wins, score from user where uid=%d;"
#define WINNER_FORMAT "update user set score=score+30, total_games=total_games+1, total_wins=total_wins+1 where uid=%d;"
#define LOSER_FORMAT "update user set score=score-30, total_games=total_games+1 where uid=%d;"

bool InfoManager::Register(const Json::Value &root) {
    char sql[4096] = { 0 };
    sprintf(sql, REGISTER_FORMAT, root["username"].asCString(), root["password"].asCString());    
    if (!UtilMysql::Execute(mysql_, sql)) {
        return false;
    }
    return true;
}

bool InfoManager::Login(Json::Value *root) {
    char sql[4096] = { 0 };
    sprintf(sql, LOGIN_FORMAT, (*root)["username"].asCString(), (*root)["password"].asCString());   
    MYSQL_RES *res = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        UtilMysql::Execute(mysql_, sql);
        res = mysql_store_result(mysql_);
    }
    if (0 == mysql_num_rows(res)) {
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    (*root)["uid"] = (Json::UInt64)std::atol(row[0]);
    (*root)["total_games"] = (Json::UInt64)std::atol(row[1]);
    (*root)["total_wins"] = (Json::UInt64)std::atol(row[2]);
    (*root)["score"] = (Json::UInt64)std::atol(row[3]);
    return true;
}

bool InfoManager::GetInfoByUsername(const std::string &username, Json::Value *root) {
    char sql[4096] = { 0 };
    sprintf(sql, GETINFOBYUSERNAME_FORMAT, username.c_str());   
    MYSQL_RES *res = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        UtilMysql::Execute(mysql_, sql);
        res = mysql_store_result(mysql_);
    }
    if (0 == mysql_num_rows(res)) {
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    (*root)["uid"] = (Json::UInt64)std::atol(row[0]);
    (*root)["username"] = username;
    (*root)["total_games"] = (Json::UInt64)std::atol(row[1]);
    (*root)["total_wins"] = (Json::UInt64)std::atol(row[2]);
    (*root)["score"] = (Json::UInt64)std::atol(row[3]);
    return true;
}

bool InfoManager::GetInfoByUid(uint64_t uid, Json::Value *root) {
    char sql[4096] = { 0 };
    sprintf(sql, GETINFOBYUID_FORMAT, uid);   
    MYSQL_RES *res = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        UtilMysql::Execute(mysql_, sql);
        res = mysql_store_result(mysql_);
    }
    if (0 == mysql_num_rows(res)) {
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    (*root)["uid"] = (Json::UInt64)uid;
    (*root)["username"] = row[0];
    (*root)["total_games"] = (Json::UInt64)std::atol(row[1]);
    (*root)["total_wins"] = (Json::UInt64)std::atol(row[2]);
    (*root)["score"] = (Json::UInt64)std::atol(row[3]);
    return true;
}

bool InfoManager::Winner(uint64_t uid) {
    char sql[4096] = { 0 };
    sprintf(sql, WINNER_FORMAT, uid);   
    if(!UtilMysql::Execute(mysql_, sql)) {
        //FIXME:即使uid是错误的，也无法进入该函数。
        LOG("Failed to update winner data.");
        return false;
    }
    return true;
}

bool InfoManager::Loser(uint64_t uid) {
    char sql[4096] = { 0 };
    sprintf(sql, LOSER_FORMAT, uid);   
    if(!UtilMysql::Execute(mysql_, sql)) {
        //FIXME:即使uid是错误的，也无法进入该函数。
        LOG("Failed to update loser data.");
        return false;
    }
    return true;
}