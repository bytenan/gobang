#ifndef __MATCH_H__
#define __MATCH_H__

#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "online.h"
#include "info.h"
#include "room.h"
#include "util.h"

class Queue {
public:
    void Push(uint64_t uid);
    bool Pop(uint64_t *uid);
    void Remove(uint64_t uid);
    size_t Size();
    bool Empty();
    void Wait();
private:
    std::list<uint64_t> list_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

class QueueManager {
public:
    QueueManager(OnlineManager *om, InfoManager *im, RoomManager *rm)
        : om_(om)
        , im_(im)
        , rm_(rm)
        , thread_bronze_(&QueueManager::ThreadBronzeHandler, this)
        , thread_silver_(&QueueManager::ThreadSilverHandler, this)
        , thread_gold_(&QueueManager::ThreadGoldHandler, this) {}
    bool Add(uint64_t uid);
    bool Del(uint64_t uid);
private:
    void MatchHandler(Queue &queue);
    void ThreadBronzeHandler();
    void ThreadSilverHandler();
    void ThreadGoldHandler();
private:
    Queue queue_bronze_;
    Queue queue_silver_;
    Queue queue_gold_;
    std::thread thread_bronze_;
    std::thread thread_silver_;
    std::thread thread_gold_;
    OnlineManager *om_;
    InfoManager *im_;
    RoomManager *rm_;
};

#endif //__MATCH_H__