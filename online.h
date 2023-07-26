#ifndef __ONLINE_H__
#define __ONLINE_H__

#include <mutex>
#include <unordered_map>

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

typedef websocketpp::server<websocketpp::config::asio> wsserver_t;

class OnlineManager {
public:
    void InHall(uint64_t uid, wsserver_t::connection_ptr &conn);
    void InRoom(uint64_t uid, wsserver_t::connection_ptr &conn);
    void OutHall(uint64_t uid);
    void OutRoom(uint64_t uid);
    bool IsinHall(uint64_t uid);
    bool IsinRoom(uint64_t uid);
    wsserver_t::connection_ptr GetConnFromHall(uint64_t uid);
    wsserver_t::connection_ptr GetConnFromRoom(uint64_t uid);
private:
    std::mutex mutex_;
    std::unordered_map<uint64_t, wsserver_t::connection_ptr> hall_;    
    std::unordered_map<uint64_t, wsserver_t::connection_ptr> room_;    
};

#endif //__ONLINE_H__