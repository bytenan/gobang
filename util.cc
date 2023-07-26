#include "util.h"

MYSQL *UtilMysql::Create(const std::string &host, int port, const std::string &user, 
            const std::string &password, const std::string &database, const std::string &csname) {
    MYSQL *mysql = mysql_init(nullptr);
    if (nullptr == mysql) {
        LOG("Failed to initialize mysql.");
        return nullptr;
    }
    if (nullptr == mysql_real_connect(mysql, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0)) {
        LOG("Failed to connect mysql client.");
        LOG("%s", mysql_error(mysql));
        mysql_close(mysql);
        return nullptr;
    }
    if(0 != mysql_set_character_set(mysql, csname.c_str())) {
        LOG("Failed to set mysql character set.");
        LOG("%s", mysql_error(mysql));
        mysql_close(mysql);
        return nullptr;
    }
    return mysql;
}

bool UtilMysql::Execute(MYSQL *mysql, const std::string &sql) {
    if (0 != mysql_query(mysql, sql.c_str())) {
        LOG("Failed to execute sql: %s.", sql.c_str());
        LOG("%s", mysql_error(mysql));
        return false;
    }
    return true;
}

void UtilMysql::Destroy(MYSQL *mysql) {
    mysql_close(mysql);
}

bool UtilJson::Serialize(const Json::Value &root, std::string *str) {
    Json::StreamWriterBuilder swb;
    std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
    std::stringstream ss;
    if (0 != sw->write(root, &ss)) {
        LOG("Failed to serialize json.");
        return false;
    }
    *str = ss.str();
    return true;
}

bool UtilJson::Parse(const std::string &str, Json::Value *root) {
    Json::CharReaderBuilder crb;
    std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
    std::string err;
    if (!cr->parse(str.c_str(), str.c_str() + str.size(), root, &err)) {
        LOG("Failed to parse string, error: %s.", err.c_str());
        return false;
    }
    return true;
}

bool UtilFile::Read(const std::string &path, std::string *body) {
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        LOG("Failed to open file: %s.", path.c_str());
        return false;
    }
    size_t size = 0;
    input.seekg(0, std::ios::end);
    size = input.tellg();
    input.seekg(0, std::ios::beg);
    body->resize(size);
    input.read(&((*body)[0]), size);
    if (!input.good()) {
        LOG("Failed to read file: %s.", path.c_str());
        return false;
    }
    input.close();
    return true;
}

size_t UtilString::Split(const std::string &src, const std::string &sep, std::vector<std::string> *dest) {
    size_t pos = 0, idx = 0;
    while (idx < src.size()) {
        pos = src.find(sep, idx);
        if (pos == std::string::npos) {
            dest->push_back(src.substr(idx));
            break;
        }
        if (idx == pos) {
            idx += sep.size();
            continue;
        }
        dest->push_back(src.substr(idx, pos - idx));
        idx = pos + sep.size();
    }
    return dest->size();
}