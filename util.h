#ifndef __UTIL_H__
#define __UTIL_H__

#include <sstream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>

#include "log.h"

namespace UtilMysql {

MYSQL *Create(const std::string &host, int port, const std::string &user, 
            const std::string &password, const std::string &database, const std::string &csname);
bool Execute(MYSQL *mysql, const std::string &sql);
void Destroy(MYSQL *mysql);

}// namespace UtilMysql

namespace UtilJson {

bool Serialize(const Json::Value &root, std::string *str);
bool Parse(const std::string &str, Json::Value *root);

}// namespace UtilJson

namespace UtilFile {

bool Read(const std::string &path, std::string *body);

}// namespace Utilfile

namespace UtilString {

size_t Split(const std::string &src, const std::string &sep, std::vector<std::string> *dest);

}// namespace UtilString


#endif //__UTIL_H__