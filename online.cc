#include "online.h"

void OnlineManager::InHall(uint64_t uid, wsserver_t::connection_ptr &conn) {
    std::unique_lock<std::mutex> lock(mutex_);
    hall_.insert(std::make_pair(uid, conn));
}

void OnlineManager::InRoom(uint64_t uid, wsserver_t::connection_ptr &conn) {
    std::unique_lock<std::mutex> lock(mutex_);
    room_.insert(std::make_pair(uid, conn));
}

void OnlineManager::OutHall(uint64_t uid) {
    std::unique_lock<std::mutex> lock(mutex_);
    hall_.erase(uid);
}

void OnlineManager::OutRoom(uint64_t uid) {
    std::unique_lock<std::mutex> lock(mutex_);
    room_.erase(uid);
}

bool OnlineManager::IsinHall(uint64_t uid) {
    std::unique_lock<std::mutex> lock(mutex_);
    return hall_.end() != hall_.find(uid);
}

bool OnlineManager::IsinRoom(uint64_t uid) {
    std::unique_lock<std::mutex> lock(mutex_);
    return room_.end() != room_.find(uid);
}

wsserver_t::connection_ptr OnlineManager::GetConnFromHall(uint64_t uid) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = hall_.find(uid);
    if (it == hall_.end()) {
        return wsserver_t::connection_ptr();
    }
    return it->second;
}

wsserver_t::connection_ptr OnlineManager::GetConnFromRoom(uint64_t uid) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = room_.find(uid);
    if (it == room_.end()) {
        return wsserver_t::connection_ptr();
    }
    return it->second;
}