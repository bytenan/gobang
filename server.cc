#include "server.h"

void GobangServer::run(int port) {
    server_.listen(port);
    server_.start_accept();
    server_.run();
}

void GobangServer::StaticResourcesHandler(wsserver_t::connection_ptr &conn) { 
    websocketpp::http::parser::request req = conn->get_request();
    std::string uri = req.get_uri();
    std::string path_name = wwwroot_ + uri;
    if (path_name == "./wwwroot/") {
        path_name = wwwroot_ + "/login.html";
    }
    std::string body;
    if (!UtilFile::Read(path_name, &body)) {
        path_name = wwwroot_ + "/404.html";
        UtilFile::Read(path_name, &body);
        conn->set_status(websocketpp::http::status_code::not_found);
    } else {
        conn->set_status(websocketpp::http::status_code::ok);
    }
    conn->set_body(body);
}

void GobangServer::HttpResponse(wsserver_t::connection_ptr &conn, const std::string &reason, 
                    const websocketpp::http::status_code::value &code) {
        Json::Value resp_json;
        resp_json["reason"] = reason;
        std::string body;
        UtilJson::Serialize(resp_json, &body);
        conn->set_status(code);
        conn->set_body(body);
        conn->append_header("Content-Type", "application/json");
}

void GobangServer::RegisterHandler(wsserver_t::connection_ptr &conn) {
    std::string req_str = conn->get_request_body();
    Json::Value req_json;
    UtilJson::Parse(req_str, &req_json);
    std::string username = req_json["username"].asString();
    std::string password = req_json["password"].asString();
    if (0 == username.size() && 0 == password.size()) {
        LOG("Please enter a username and password.");
        return HttpResponse(conn, "Please enter a username and password.", websocketpp::http::status_code::bad_request);
    } else if (0 == username.size()) {
        LOG("Please enter a username.");
        return HttpResponse(conn, "Please enter a username.", websocketpp::http::status_code::bad_request);
    } else if (0 == password.size()) {
        LOG("Please enter a password.");
        return HttpResponse(conn, "Please enter a password.", websocketpp::http::status_code::bad_request);
    } else if (6 > password.size()) {
        LOG("Password length is less than 6, please re-enter.");
        return HttpResponse(conn, "Password length is less than 6, please re-enter.", websocketpp::http::status_code::bad_request);
    }
    if(!im_.Register(req_json)) {
        LOG("Username already exists.");
        return HttpResponse(conn, "Username already exists.", websocketpp::http::status_code::bad_request);
    }
    LOG("Registration successful.");
    HttpResponse(conn, "Registration successful.", websocketpp::http::status_code::ok);
}

void GobangServer::LoginHandler(wsserver_t::connection_ptr &conn) {
    std::string req_str = conn->get_request_body();
    Json::Value req_json;
    UtilJson::Parse(req_str, &req_json);
    std::string username = req_json["username"].asString();
    std::string password = req_json["password"].asString();
    if (0 == username.size() && 0 == password.size()) {
        LOG("Please enter a username and password.");
        return HttpResponse(conn, "Please enter a username and password.", websocketpp::http::status_code::bad_request);
    } else if (0 == username.size()) {
        LOG("Please enter a username.");
        return HttpResponse(conn, "Please enter a username.", websocketpp::http::status_code::bad_request);
    } else if (0 == password.size()) {
        LOG("Please enter a password.");
        return HttpResponse(conn, "Please enter a password.", websocketpp::http::status_code::bad_request);
    } 
    if (!im_.Login(&req_json)) {
        LOG("Username or password is incorrect.");
        return HttpResponse(conn, "Username or password is incorrect.", websocketpp::http::status_code::bad_request);
    }
    uint64_t uid = req_json["uid"].asUInt64();
    if (om_.IsInRoom(uid) || om_.IsInHall(uid)) {
        LOG("Do not log in repeatedly.");
        return HttpResponse(conn, "Do not log in repeatedly.", websocketpp::http::status_code::bad_request);
    }
    SessionPtr ssp = sm_.CreateSession(uid, SESSION_LOGIN);
    if (nullptr == ssp.get()) {
        LOG("Create session fail.");
        return HttpResponse(conn, "Create session fail.", websocketpp::http::status_code::bad_request);
    }
    sm_.SetSessionExpireTime(ssp->ssid(), SESSION_FOROVER);
    std::string cookie_ssid = "SSID=" + std::to_string(ssp->ssid());
    conn->append_header("Set-Cookie", cookie_ssid);
    LOG("Login successful.");
    HttpResponse(conn, "Login successful.", websocketpp::http::status_code::ok);
}

std::string GobangServer::GetValFromCookie(const std::string &cookie, const std::string &key) {
    std::vector<std::string> cookie_array;
    UtilString::Split(cookie, "; ", &cookie_array);
    std::vector<std::string> temp;
    for (auto &str : cookie_array) {
        temp.resize(0);
        UtilString::Split(str, "=", &temp);
        if (temp.size() != 2) continue;
        if (temp[0] == key) {
            break;
        }
    }
    return temp[1];
}

void GobangServer::InfoHandler(wsserver_t::connection_ptr &conn) {
    std::string cookie = conn->get_request_header("Cookie");
    if (cookie.empty()) {
        LOG("There are no cookies, please log in again.");
        return HttpResponse(conn, "There are no cookies, please log in again.", websocketpp::http::status_code::internal_server_error); 
    }
    std::string ssid = GetValFromCookie(cookie, "SSID");
    SessionPtr ssp = sm_.SessionBySsid(std::atol(ssid.c_str()));
    Json::Value info_json;
    im_.GetInfoByUid(ssp->uid(), &info_json);
    std::string body;
    UtilJson::Serialize(info_json, &body);
    conn->set_body(body);
    conn->set_status(websocketpp::http::status_code::ok);
    conn->append_header("Content-Type", "application/json");
    sm_.SetSessionExpireTime(ssp->ssid(), SESSION_TIMEOUT);
}

void GobangServer::HttpHandler(websocketpp::connection_hdl hdl) {
    wsserver_t::connection_ptr conn = server_.get_con_from_hdl(hdl);
    websocketpp::http::parser::request req = conn->get_request();
    std::string method = req.get_method();
    std::string uri = req.get_uri();
    if ("POST" == method && "/register" == uri) {
        RegisterHandler(conn);
    } else if ("POST" == method && "/login" == uri) {
        LoginHandler(conn);
    } else if ("GET" == method && "/info" == uri) {
        InfoHandler(conn);
    } else {
        StaticResourcesHandler(conn);
    }
}

void WsResponse(wsserver_t::connection_ptr &conn, const std::string &type, const std::string &result) {
    Json::Value resp_json;
    resp_json["optype"] = type;
    resp_json["result"] = result;
    std::string body;
    UtilJson::Serialize(resp_json, &body);
    conn->send(body);
}

void GobangServer::OpenHallHandler(wsserver_t::connection_ptr &conn){
    std::string cookie = conn->get_request_header("Cookie");
    std::string ssid = GetValFromCookie(cookie, "SSID");
    SessionPtr ssp = sm_.SessionBySsid(std::atol(ssid.c_str()));
    om_.InHall(ssp->uid(), conn);
    sm_.SetSessionExpireTime(ssp->ssid(), SESSION_FOROVER);
    LOG("Successfully enter the game lobby.");
    WsResponse(conn, "hall_ready", "Successfully enter the game lobby.");
}
void GobangServer::OpenRoomHandler(wsserver_t::connection_ptr &conn){
    //TODO:
    std::string cookie = conn->get_request_header("Cookie");
    std::string ssid = GetValFromCookie(cookie, "SSID");
    SessionPtr ssp = sm_.SessionBySsid(std::atol(ssid.c_str()));
    om_.InRoom(ssp->uid(), conn);
    sm_.SetSessionExpireTime(ssp->ssid(), SESSION_FOROVER);
    LOG("Successfully enter the game room.");
    RoomPtr rp = rm_.RoomByUid(ssp->uid());
    Json::Value resp_json;
    resp_json["optype"] = "room_ready";
    resp_json["result"] = "Successfully enter the game room.";
    resp_json["rid"] = (Json::UInt64)rp->rid();
    resp_json["uid"] = (Json::UInt64)ssp->uid();
    resp_json["white_id"] = (Json::UInt64)rp->white_id();
    resp_json["black_id"] = (Json::UInt64)rp->black_id();
    std::string body;
    UtilJson::Serialize(resp_json, &body);
    conn->send(body);
}
void GobangServer::OpenHandler(websocketpp::connection_hdl hdl) {
    wsserver_t::connection_ptr conn = server_.get_con_from_hdl(hdl);
    websocketpp::http::parser::request req = conn->get_request();
    std::string uri = req.get_uri();
    if (uri == "/hall") {
        return OpenHallHandler(conn);
    } else if (uri == "/room") {
        return OpenRoomHandler(conn);
    }
}

void GobangServer::CloseHallHandler(wsserver_t::connection_ptr &conn){
    std::string cookie = conn->get_request_header("Cookie");
    std::string ssid = GetValFromCookie(cookie, "SSID");
    SessionPtr ssp = sm_.SessionBySsid(std::atol(ssid.c_str()));
    om_.OutHall(ssp->uid());
    sm_.SetSessionExpireTime(ssp->ssid(), SESSION_TIMEOUT);
}
void GobangServer::CloseRoomHandler(wsserver_t::connection_ptr &conn){
    //TODO:
    std::string cookie = conn->get_request_header("Cookie");
    std::string ssid = GetValFromCookie(cookie, "SSID");
    SessionPtr ssp = sm_.SessionBySsid(std::atol(ssid.c_str()));
    om_.OutRoom(ssp->uid());
    rm_.RemoveUserFromRoom(ssp->uid());
    sm_.SetSessionExpireTime(ssp->ssid(), SESSION_TIMEOUT);
}
void GobangServer::CloseHandler(websocketpp::connection_hdl hdl) {
    wsserver_t::connection_ptr conn = server_.get_con_from_hdl(hdl);
    websocketpp::http::parser::request req = conn->get_request();
    std::string uri = req.get_uri();
    if (uri == "/hall") {
        return CloseHallHandler(conn);
    } else if (uri == "/room") {
        return CloseRoomHandler(conn);
    }
}

void GobangServer::MessageHallHandler(wsserver_t::connection_ptr &conn, wsserver_t::message_ptr msg) {
    std::string cookie = conn->get_request_header("Cookie");
    std::string ssid = GetValFromCookie(cookie, "SSID");
    SessionPtr ssp = sm_.SessionBySsid(std::atol(ssid.c_str()));
    std::string req_str = msg->get_payload();
    Json::Value req_json;
    UtilJson::Parse(req_str, &req_json);
    if ("match_start" == req_json["optype"].asString()) {
        qm_.Add(ssp->uid());
        return WsResponse(conn, "match_start", "Start matching");
    } else if ("match_stop" == req_json["optype"].asString()) {
        qm_.Del(ssp->uid());
        return WsResponse(conn, "match_stop", "Stop matching");
    }
}
void GobangServer::MessageRoomHandler(wsserver_t::connection_ptr &conn, wsserver_t::message_ptr msg) {
    //TODO:
    std::string req_str = msg->get_payload();
    Json::Value req_json;
    UtilJson::Parse(req_str, &req_json);
    RoomPtr rp = rm_.RoomByRid(req_json["rid"].asUInt64());
    rp->RequestHandler(req_json);
}
void GobangServer::MessageHandler(websocketpp::connection_hdl hdl, wsserver_t::message_ptr msg) {
    wsserver_t::connection_ptr conn = server_.get_con_from_hdl(hdl);
    websocketpp::http::parser::request req = conn->get_request();
    std::string uri = req.get_uri();
    if (uri == "/hall") {
        return MessageHallHandler(conn, msg);
    } else if (uri == "/room") {
        return MessageRoomHandler(conn, msg);
    }
}