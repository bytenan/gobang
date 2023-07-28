#include "match.h"

void Queue::Push(uint64_t uid) {
    std::unique_lock<std::mutex> lock(mutex_);
    list_.push_back(uid);
    cond_.notify_all();
}

bool Queue::Pop(uint64_t *uid) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (list_.empty()) {
        return false;
    }
    *uid = list_.front();
    list_.pop_front();
    return true;
}

void Queue::Remove(uint64_t uid) {
    std::unique_lock<std::mutex> lock(mutex_);
    list_.remove(uid);
}

size_t Queue::Size() {
    std::unique_lock<std::mutex> lock(mutex_);
    return list_.size();
}

bool Queue::Empty() {
    std::unique_lock<std::mutex> lock(mutex_);
    return list_.empty();
}

void Queue::Wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock);
}

bool QueueManager::Add(uint64_t uid) {
    Json::Value info;
    if (!im_->GetInfoByUid(uid, &info)) {
        return false;
    }
    int score = info["score"].asInt();
    if (score < 2000) {
        queue_bronze_.Push(uid);
    } else if (2000 <= score && score < 3000) {
        queue_silver_.Push(uid);
    } else if (3000 <= score) {
        queue_gold_.Push(uid);
    }
    return true;
}

bool QueueManager::Del(uint64_t uid) {
    Json::Value info;
    if (!im_->GetInfoByUid(uid, &info)) {
        return false;
    }
    int score = info["score"].asInt();
    if (score < 2000) {
        queue_bronze_.Remove(uid);
    } else if (2000 <= score && score < 3000) {
        queue_silver_.Remove(uid);
    } else if (3000 <= score) {
        queue_gold_.Remove(uid);
    }
    return true;
}

void QueueManager::MatchHandler(Queue &queue) {
    while (true) {
        while (queue.Size() < 2) {
            queue.Wait();
        }
        uint64_t uid1, uid2;
        if(!queue.Pop(&uid1)) {
            continue;
        }
        if (!queue.Pop(&uid2)) {
            this->Add(uid1);
            continue;
        }
        wsserver_t::connection_ptr conn1 = om_->GetConnFromHall(uid1);
        if(nullptr == conn1.get()) {
            this->Add(uid2);            
            continue;
        }
        wsserver_t::connection_ptr conn2 = om_->GetConnFromHall(uid2);
        if(nullptr == conn2.get()) {
            this->Add(uid1);            
            continue;
        }
        RoomPtr rp = rm_->CreateRoom(uid1, uid2);
        if (nullptr == rp.get()) {
            this->Add(uid1);
            this->Add(uid2);
            continue;
        }
        Json::Value resp;
        resp["optype"] = "match_success";
        resp["result"] = true;
        std::string body;
        UtilJson::Serialize(resp, &body);
        conn1->send(body);
        conn2->send(body);
    }
}

void QueueManager::ThreadBronzeHandler() {
    MatchHandler(queue_bronze_);
}

void QueueManager::ThreadSilverHandler() {
    MatchHandler(queue_silver_);
}

void QueueManager::ThreadGoldHandler() {
    MatchHandler(queue_gold_);
}