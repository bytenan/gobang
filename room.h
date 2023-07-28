#ifndef __ROOM_H__
#define __ROOM_H__

#include <vector>
#include <memory>
#include <unordered_map>

#include <jsoncpp/json/json.h>

#include "util.h"
#include "info.h"
#include "online.h"

#define BOARD_ROW 15
#define BOARD_COL 15
typedef enum { GAME_START, GAME_OVER } RoomStatu;

class Room {
public:
    Room(uint64_t rid, InfoManager *im, OnlineManager *om)
        : rid_(rid)
        , white_id_(-1)
        , black_id_(-1)
        , total_players_(0)
        , statu_(GAME_START)
        , im_(im)
        , om_(om)
        , board_(std::vector<std::vector<int>>(BOARD_ROW, std::vector<int>(BOARD_COL, 0))) {}
    uint64_t rid();
    uint64_t white_id();
    uint64_t black_id();
    uint64_t total_players();
    void set_white_id(uint64_t white_id);
    void set_black_id(uint64_t black_id);
    Json::Value ChessHandler(const Json::Value &req);
    Json::Value ChatHandler(const Json::Value &req);
    void RequestHandler(const Json::Value &req);
    void ExitHandler(uint64_t uid);
    void Boardcast(const Json::Value &resp);
private:
    uint64_t WinnerIdIfHas(int row, int col, int color);
    bool IsWin(int row, int col, int row_offset, int col_offset, int color);
private:
    uint64_t rid_;
    uint64_t white_id_;
    uint64_t black_id_;
    uint32_t total_players_;
    RoomStatu statu_;
    InfoManager *im_;
    OnlineManager *om_;
    std::vector<std::vector<int>> board_;
};

typedef std::shared_ptr<Room> RoomPtr;
class RoomManager {
public:
    RoomManager(InfoManager *im, OnlineManager *om)
        : next_rid_(1)
        , im_(im)
        , om_(om) {}
    RoomPtr CreateRoom(uint64_t uid1, uint64_t uid2);
    RoomPtr RoomByRid(uint64_t rid);
    RoomPtr RoomByUid(uint64_t uid);
    void DestoryRoomByRid(uint64_t rid);
    void RemoveUserFromRoom(uint64_t uid);
private:
    std::mutex mutex_;
    uint64_t next_rid_;
    std::unordered_map<uint64_t, RoomPtr> rooms_;
    std::unordered_map<uint64_t, uint64_t> rids_;
    InfoManager *im_;
    OnlineManager *om_;
};

#endif //__ROOM_H__