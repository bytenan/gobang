#include "room.h"

uint64_t Room::rid() {
    return rid_; 
}

uint64_t Room::white_id() { 
    return white_id_; 
}

uint64_t Room::black_id() { 
    return black_id_; 
}

uint64_t Room::total_players() { 
    return total_players_; 
}

void Room::set_white_id(uint64_t white_id) {
    white_id_ = white_id;
    ++total_players_; 
}

void Room::set_black_id(uint64_t black_id) { 
    black_id_ = black_id; 
    ++total_players_; 
}

Json::Value Room::ChessHandler(const Json::Value &req) {
    Json::Value resp;
    resp["optype"] = "chess";
    int row = req["row"].asInt();
    int col = req["col"].asInt();
    if (!(0 <= row && row < BOARD_ROW && 0 <= col && col < BOARD_COL)) {
        resp["result"] = false;
        resp["reason"] = "该位置不允许下棋！";
        resp["winner_id"] = 0;
        return resp;
    }
    if (0 != board_[row][col]) {
        resp["result"] = false;
        resp["reason"] = "该位置已有棋子！";
        resp["winner_id"] = 0;
        return resp;
    }
    int color = board_[row][col] = (req["uid"].asUint64() == black_id_ ? black_id_ : white_id_);
    uint64_t winnner_id = WinnerIdIfHas(row, col, color);
    if (0 != winner_id) {
        resp["reason"] = "五星连珠，战无敌！";
    }
    resp["result"] = true;
    resp["winner_id"] = (Json::UInt64)winnner_id;
    return resp;
}

Json::Value Room::ChatHandler(const Json::Value &req) {
    Json::Value resp;
    resp["optype"] = "chat";
    std::string message = req["message"].asString();
    if (std::string::npos != message.find("垃圾")) {
        resp["result"] = false;
        resp["reason"] = "存在敏感词汇";
        return resp;
    }
    resp["result"] = true;
    return resp;
}

void Room::RequestHandler(const Json::Value &req) {
    Json::Value resp;
    if ("chess" == req["optype"]) {
        resp = ChessHandler(req);
        winner_id = resp["winner_id"].asUInt64();
        if (0 != winner_id) {
            im_->Winner(winner_id == white_id_ ? white_id_ : black_id_);
            im_->Loser(winner_id == white_id_ ? black_id_ : white_id_);
        }
    } else if ("chat" == req["optype"]) {
        resp = ChatHandler(req);
    } else {
        resp["optype"] = "unknow";
        resp["result"] = false;
        resp["reason"] = "未知操作类型";
    }
    Boardcast(resp);
}

void Room::ExitHandler(uint64_t uid) {
    Json::Value resp;
    if (GAME_START == statu_) {
        uint64_t winner_id = uid == white_id_ ? black_id_ : white_id_;
        uint64_t loser_id = uid == white_id_ ? white_id_ : black_id_;
        im_->Winner(winner_id);
        im_->Loser(loser_id);
        statu_ = GAME_OVER;
        resp["result"] = true;
        resp["reason"] = "对方离线，不战而胜";
        resp["winner_id"] = (Json::UInt64)winner_id;
        Boardcast(resp);
    }
    --total_players_;
    return;
}

void Room::Boardcast(const Json::Value &resp) {
    std::string body;
    UtilJson::Serialize(resp, &body);
    wsserver_t::connection_ptr white_conn = om_->GetConnFromRoom(white_id_);
    wsserver_t::connection_ptr black_conn = om_->GetConnFromRoom(black_id_);
    if (nullptr != white_conn.get()) white_conn->send(body);
    if (nullptr != black_conn.get()) black_conn->send(body);
}

uint64_t Room::WinnerIdIfHas(int row, int col, int color) {
    if (IsWin(row, col, 0, 1, color) || IsWin(row, col, 1, 0, color) || 
        IsWin(row, col, 1, 1, color) || IsWin(row, col, 1, -1, color)) {
        return color == white_id_ ? white_id_ : black_id_;
    }
    return 0;
}

bool Room::IsWin(int row, int col, int row_offset, int col_offset, int color) {
    int count = 0;
    int search_row = row;
    int search_col = col;
    while (0 <= search_row && search_row < BOARD_ROW && 
           0 <= search_col && search_col < BOARD_COL && 
           color == board_[search_row][search_col]) {
        search_row += row_offset;
        search_col += col_offset;
        ++count;
    }
    search_row = row;
    search_col = col;
    while (0 <= search_row && search_row < BOARD_ROW && 
           0 <= search_col && search_col < BOARD_COL && 
           color == board_[search_row][search_col]) {
        search_row -= row_offset;
        search_col -= col_offset;
        ++count;
    }
    return count >= 5;
}

RoomPtr RoomManager::CreateRoom(uint64_t uid1, uint64_t uid2) {
    if(!om_->IsinHall(uid1)) {
        LOG("Uid1 %lu is not in hall.", uid1);
        return RoomPtr();
    }
    if(!om_->IsinHall(uid2)) {
        LOG("Uid2 %lu is not in hall.", uid2);
        return RoomPtr();
    }
    std::unique_lock(std::mutex) lock(mutex_);
    RoomPtr rp(new Room(next_rid_, im_, om_));
    rp->set_black_id(uid1);
    rp->set_white_id(uid2);
    rooms_.insert(std::make_pair(next_rid_, rp));
    rids_.insert(std::make_pair(uid1, next_rid_));
    rids_.insert(std::make_pair(uid2, next_rid_));
    ++next_rid_;
    return rp;
}

RoomPtr RoomManager::RoomByRid(uint64_t rid) {
    std::unique_lock(std::mutex) lock(mutex_);
    auto it = rooms_.find(uid);
    if (rooms_.end() == it) {
        return RoomPtr();
    }
    return it->second;
}

RoomPtr RoomManager::RoomByUid(uint64_t uid) {
    std::unique_lock(std::mutex) lock(mutex_);
    auto rid_it = rids_.find(uid);
    if (rids_.end() == rid_it) {
        return RoomPtr();
    }
    auto room_it = rooms_.find(rid_it->second);
    if (rooms_.end() == room_it) {
        return RoomPtr();
    }
    return room_it->second;
}

void RoomManager::DestoryRoomByRid(uint64_t rid) {
    RoomPtr rp = RoomByRid(rid);
    if (nullptr == rp.get()) {
        return;
    }
    std::unique_lock(std::mutex) lock(mutex_);
    rids_.erase(rp->white_id());
    rids_.erase(rp->black_id());
    rooms_.erase(rid);
}

void RoomManager::RemoveUserFromRoom(uint64_t uid) {
    RoomPtr rp = RoomByRid(rid);
    if (nullptr == rp.get()) {
        return;
    }
    rp->ExitHandler(uid);
    if (0 == rp->total_players()) {
        DestoryRoomByRid(rp->rid());
    }
}