#ifndef __SESSION_H__
#define __SESSION_H__

#include <mutex>
#include <memory>
#include <unordered_map>

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

typedef websocketpp::server<websocketpp::config::asio> wsserver_t;
typedef enum { SESSION_LOGIN, SESSION_UNLOGIN } SessionStatu;

class Session {
public:
    Session(uint64_t ssid) : ssid_(ssid) {}
    uint64_t ssid();
    uint64_t uid();
    void set_uid(uint64_t uid);
    void set_statu(SessionStatu statu);
    bool IsLogin();
    void set_timer(const wsserver_t::timer_ptr &timer);
    wsserver_t::timer_ptr &timer();
private:
    uint64_t ssid_;
    uint64_t uid_;
    SessionStatu statu_;
    wsserver_t::timer_ptr timer_;
};

#define SESSION_TIMEOUT 30000 
#define SESSION_FOROVER -1
typedef std::shared_ptr<Session> SessionPtr;
class SessionManager {
public:
    SessionManager(wsserver_t *server) : next_ssid_(1), server_(server) {}
    SessionPtr CreateSession(uint64_t uid, SessionStatu statu);
    void AppendSession(const SessionPtr &ssp);
    void RemoveSession(uint64_t ssid);
    SessionPtr SessionBySsid(uint64_t ssid);
    void SetSessionExpireTime(uint64_t ssid, int ms);
private:
    uint64_t next_ssid_;
    std::mutex mutex_;
    std::unordered_map<uint64_t, SessionPtr> sessions_;
    wsserver_t *server_;
};

#endif //__SESSION_H__