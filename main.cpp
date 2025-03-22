#define CROW_ENABLE_WEBSOCKET
#include "crow.h"
#include "crow/websocket.h"
#include <vector>
#include <random>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <mutex>

using namespace std;

struct Player {
    string name;
    string role;
    bool alive = true;
    bool ready = false;
};

struct CORSHandler : crow::ILocalMiddleware {
    struct context {};
    void before_handle(crow::request& req, crow::response& res, context&) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
    }
    void after_handle(crow::request&, crow::response&, context&) {}
};

struct GameRoom {
    vector<Player> players;
    bool gameStarted = false;
    int nightPhase = 0;
    unordered_map<string, string> actions;
    vector<crow::websocket::connection*> connections;
};

unordered_map<string, GameRoom> rooms;
mutex roomMutex;

vector<string> assignRoles(int playerCount) {
    vector<string> assignedRoles;
    int numWolves = max(1, playerCount / 4);
    int numSeer = 1, numGuard = 1, numWitch = 1;
    int numVillagers = playerCount - (numWolves + numSeer + numGuard + numWitch);

    for (int i = 0; i < numWolves; i++) assignedRoles.push_back("Sói");
    for (int i = 0; i < numSeer; i++) assignedRoles.push_back("Tiên tri");
    for (int i = 0; i < numGuard; i++) assignedRoles.push_back("Bảo vệ");
    for (int i = 0; i < numWitch; i++) assignedRoles.push_back("Phù thủy");
    for (int i = 0; i < numVillagers; i++) assignedRoles.push_back("Dân làng");

    shuffle(assignedRoles.begin(), assignedRoles.end(), mt19937(random_device()()));
    return assignedRoles;
}

void broadcast(const string& roomID, const string& message) {
    lock_guard<mutex> lock(roomMutex);
    if (rooms.find(roomID) == rooms.end()) return;

    for (auto* conn : rooms[roomID].connections) {
        if (conn) conn->send_text(message);
    }
}

int main() {
    crow::App<CORSHandler> app;

    // Phục vụ favicon
    CROW_ROUTE(app, "/favicon.ico")([]() {
        return crow::response(204);
    });

    // Phục vụ file index.html
    CROW_ROUTE(app, "/")([]() {
        ifstream file("src/index.html");
        if (!file) return crow::response(404, "Không tìm thấy file index.html");
        return crow::response(string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>()));
    });

    // Phục vụ file tĩnh (CSS, JS)
    CROW_ROUTE(app, "/static/<string>")([](string filename) {
        string filepath = "src/" + filename;
        if (filename.find("..") != string::npos || (filename.find(".js") == string::npos && filename.find(".css") == string::npos)) {
            return crow::response(403, "Truy cập bị từ chối");
        }
        ifstream file(filepath);
        if (!file) return crow::response(404, "Không tìm thấy file: " + filename);
        crow::response res(string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>()));
        if (filename.find(".css") != string::npos) res.set_header("Content-Type", "text/css");
        if (filename.find(".js") != string::npos) res.set_header("Content-Type", "application/javascript");
        return res;
    });

    // WebSocket: Kết nối với phòng
    CROW_ROUTE(app, "/ws/<string>")
.websocket(&app)
.onopen([&](crow::websocket::connection& conn) {
    cout << "WebSocket kết nối!" << endl;

    lock_guard<mutex> lock(roomMutex);
    string roomID = "room1";  // Thay thế bằng cách lấy từ một nơi khác nếu cần
    if (rooms.find(roomID) == rooms.end()) {
        rooms[roomID] = GameRoom();
    }
    rooms[roomID].connections.push_back(&conn);
})
.onclose([&](crow::websocket::connection& conn, uint16_t code) {
    cout << "WebSocket đóng kết nối!" << endl;

    lock_guard<mutex> lock(roomMutex);
    string roomID = "room1";  // Cần có cách lấy `roomID` từ `conn`
    if (rooms.find(roomID) != rooms.end()) {
        auto& conns = rooms[roomID].connections;
        conns.erase(remove(conns.begin(), conns.end(), &conn), conns.end());
        if (conns.empty()) {
            rooms.erase(roomID);
        }
    }
});

    

    // API tham gia phòng
    CROW_ROUTE(app, "/join-game").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("name") || !body.has("roomID")) return crow::response(400, "Thiếu thông tin");

        string playerName = body["name"].s();
        string roomID = body["roomID"].s();

        lock_guard<mutex> lock(roomMutex);
        if (rooms.find(roomID) == rooms.end()) rooms[roomID] = GameRoom();
        rooms[roomID].players.push_back({playerName, "", true, false});
        broadcast(roomID, playerName + " đã tham gia phòng!");

        return crow::response(200);
    });

    // API bắt đầu game
    CROW_ROUTE(app, "/start-game").methods("POST"_method)([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("roomID")) return crow::response(400, "Thiếu roomID");

        string roomID = body["roomID"].s();
        lock_guard<mutex> lock(roomMutex);

        if (rooms.find(roomID) == rooms.end() || rooms[roomID].gameStarted || rooms[roomID].players.size() < 4)
            return crow::response(400, "Không thể bắt đầu game");

        rooms[roomID].gameStarted = true;
        broadcast(roomID, "Trò chơi đã bắt đầu!");
        return crow::response(200);
    });

    app.port(18080).multithreaded().run();
}
