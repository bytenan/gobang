#include <iostream>

// #include "info.h"

// #include "online.h"

// #include "room.h"

// #include "session.h"

// #include "match.h"

#include "server.h"

// void TestMysql() {
//     MYSQL *mysql = UtilMysql::Create(HOST, PORT, USER, PASSWORD, DATABASE, CSNAME);
//     std::string sql = "insert user values (null, 'bytenan', password('123456'), 1000, 0, 0)";
//     UtilMysql::Execute(mysql, sql);
//     UtilMysql::Destroy(mysql);
// }

// void TestJson() {
//     Json::Value root;
//     root["username"] = "王亚楠";
//     root["password"] = "gawfawfa";
//     std::string str;
//     UtilJson::Serialize(root, &str);
//     std::cout << str << std::endl;
//     Json::Value json;
//     UtilJson::Parse(str, &json);
//     std::cout << "username: " << json["username"] << std::endl;
//     std::cout << "password: " << json["password"] << std::endl;
// }

// void TestFile() {
//     std::string body;
//     UtilFile::Read("./logger.h", &body);
//     std::cout << body << std::endl;
// }

// void TestString() {
//     std::string str = "...dwada.daw...dagrd..grwa..";
//     std::vector<std::string> dest;
//     UtilString::Split(str, ".", &dest);
//     for (auto &str : dest) {
//         std::cout << str << std::endl;
//     }
// }

// void TestInfo() {
//     InfoManager im;
//     // Json::Value root;
//     // root["username"] = "xiaowang";
//     // root["password"] = "123456";
//     // im.Register(root);

//     // im.Login(&root);
//     // std::cout << root["uid"] << std::endl;
//     // std::cout << root["total_games"] << std::endl;
//     // std::cout << root["total_wins"] << std::endl;
//     // std::cout << root["score"] << std::endl;
    
//     // std::string username = "xiaowang";
//     // Json::Value dest;
//     // im.GetInfoByUsername(username, &dest);
//     // std::cout << dest["uid"] << std::endl;
//     // std::cout << dest["username"] << std::endl;
//     // std::cout << dest["total_games"] << std::endl;
//     // std::cout << dest["total_wins"] << std::endl;
//     // std::cout << dest["score"] << std::endl;

//     // uint64_t uid = 4;
//     // Json::Value dest;
//     // im.GetInfoByUid(uid, &dest);
//     // std::cout << dest["uid"] << std::endl;
//     // std::cout << dest["username"] << std::endl;
//     // std::cout << dest["total_games"] << std::endl;
//     // std::cout << dest["total_wins"] << std::endl;
//     // std::cout << dest["score"] << std::endl;

//     uint64_t uid = 4;
//     // im.Winner(uid);
//     im.Loser(uid);
// }

int main() {

    // TestInfo();

    GobangServer gs;
    gs.run(8080);

    return 0;
}