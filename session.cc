#include "session.h"

uint64_t Session::ssid() {
    return ssid_;
}

uint64_t Session::uid() {
    return uid_;
}

void Session::set_uid(uint64_t uid) {
    uid_ = uid;
}

void Session::set_statu(SessionStatu statu) {
    statu_ = statu;
}

bool Session::IsLogin() {
    return statu_ == SESSION_LOGIN;
}

void Session::set_timer(const wsserver_t::timer_ptr &timer) {
    timer_ = timer;
}

wsserver_t::timer_ptr &Session::timer() {
    return timer_;
}

SessionPtr SessionManager::CreateSession(uint64_t uid, SessionStatu statu) {
    std::unique_lock<std::mutex> lock(mutex_);
    SessionPtr ssp(new Session(next_ssid_));
    ssp->set_uid(uid);
    ssp->set_statu(statu);
    sessions_.insert(std::make_pair(next_ssid_, ssp));
    ++next_ssid_;
    return ssp;
}

void SessionManager::AppendSession(const SessionPtr &ssp) {
    std::unique_lock<std::mutex> lock(mutex_);
    sessions_.insert(std::make_pair(ssp->ssid(), ssp));
}

void SessionManager::RemoveSession(uint64_t ssid) {
    std::unique_lock<std::mutex> lock(mutex_);
    sessions_.erase(ssid);
}

SessionPtr SessionManager::SessionBySsid(uint64_t ssid) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = sessions_.find(ssid);
    if (sessions_.end() == it) {
        return SessionPtr();
    }
    return it->second;
}

void SessionManager::SetSessionExpireTime(uint64_t ssid, int ms) {
    SessionPtr ssp = SessionBySsid(ssid);
    if (nullptr == ssp.get()) {
        return;
    }
    wsserver_t::timer_ptr timer = ssp->timer();
    if (nullptr == timer.get() && SESSION_FOROVER == ms) {
        return;
    } else if (nullptr == timer.get() && SESSION_FOROVER != ms) {
        ssp->set_timer(server_->set_timer(ms, std::bind(&SessionManager::RemoveSession, this, ssid)));
    } else if (nullptr != timer.get() && SESSION_FOROVER == ms) {
        timer->cancel();
        ssp->set_timer(wsserver_t::timer_ptr());
        server_->set_timer(0, std::bind(&SessionManager::AppendSession, this, ssp));
    } else if (nullptr != timer.get() && SESSION_FOROVER != ms) {
        timer->cancel();
        ssp->set_timer(wsserver_t::timer_ptr());
        server_->set_timer(0, std::bind(&SessionManager::AppendSession, this, ssp));
        ssp->set_timer(server_->set_timer(ms, std::bind(&SessionManager::RemoveSession, this, ssid)));
    }
}