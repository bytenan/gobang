#ifndef __SERVER_H__
#define __SERVER_H__

#include "log.h"
#include "util.h"
#include "info.h"
#include "room.h"
#include "match.h"
#include "online.h"
#include "session.h"

#define WWWROOT "./wwwroot"

#define HOST "127.0.0.1"
#define PORT 3306
#define USER "root"
#define PASSWORD "Wang2923944542"
#define DATABASE "gobang"
#define CSNAME "utf8"

class GobangServer {
public:
    GobangServer()
        : wwwroot_(WWWROOT)
        , sm_(&server_)
        , im_(HOST, PORT, USER, PASSWORD, DATABASE, CSNAME)
        , rm_(&im_, &om_)
        , qm_(&om_, &im_, &rm_) {
        server_.set_access_channels(websocketpp::log::alevel::none);
        server_.init_asio();
        server_.set_reuse_addr(true);
        server_.set_http_handler(bind(&GobangServer::HttpHandler, this, std::placeholders::_1));
        server_.set_open_handler(bind(&GobangServer::OpenHandler, this, std::placeholders::_1));
        server_.set_close_handler(bind(&GobangServer::CloseHandler, this, std::placeholders::_1));
        server_.set_message_handler(bind(&GobangServer::MessageHandler, this, std::placeholders::_1, std::placeholders::_2));
    }
    void run(int port);
private:
    void StaticResourcesHandler(wsserver_t::connection_ptr &conn);
    void HttpResponse(wsserver_t::connection_ptr &conn, const std::string &reason, const websocketpp::http::status_code::value &code);
    void RegisterHandler(wsserver_t::connection_ptr &conn);
    void LoginHandler(wsserver_t::connection_ptr &conn);
    std::string GetValFromCookie(const std::string &cookie, const std::string &key);
    void InfoHandler(wsserver_t::connection_ptr &conn);
    void HttpHandler(websocketpp::connection_hdl hdl);

    void OpenHallHandler(wsserver_t::connection_ptr &conn);
    void OpenRoomHandler(wsserver_t::connection_ptr &conn);
    void OpenHandler(websocketpp::connection_hdl hdl);

    void CloseHallHandler(wsserver_t::connection_ptr &conn);
    void CloseRoomHandler(wsserver_t::connection_ptr &conn);

    void MessageHallHandler(wsserver_t::connection_ptr &conn);
    void MessageRoomHandler(wsserver_t::connection_ptr &conn);
    void CloseHandler(websocketpp::connection_hdl hdl);
    void MessageHandler(websocketpp::connection_hdl hdl, wsserver_t::message_ptr msg);
private:
    wsserver_t server_;
    std::string wwwroot_;
    SessionManager sm_;
    OnlineManager om_;
    InfoManager im_;
    RoomManager rm_;
    QueueManager qm_;
};

#endif //__SERVER_H__