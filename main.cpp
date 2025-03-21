#include <crow.h>
#include <vector>
#include <random>
#include <fstream>

using namespace std;

vector<string> roles = {"Dân làng", "Sói", "Bảo vệ", "Tiên tri", "Phù thủy"};

vector<string> assignRoles(int playerCount) {
    vector<string> assignedRoles;
    for (int i = 0; i < playerCount; i++) {
        assignedRoles.push_back(roles[i % roles.size()]); // Xoay vòng vai trò
    }
    shuffle(assignedRoles.begin(), assignedRoles.end(), mt19937(random_device()()));
    return assignedRoles;
}

// Hàm đọc file HTML
string loadFile(const string& filename) {
    ifstream file(filename); // Đọc file theo tham số truyền vào
    if (!file) return "Lỗi: Không tìm thấy file: " + filename;
    return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}


int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/assign-roles")
    .methods(crow::HTTPMethod::GET)([](const crow::request& req) {
        auto query = crow::query_string(req.url_params);
        if (!query.get("count")) {
            return crow::response(400, "Thiếu tham số count");
        }

        int playerCount = stoi(query.get("count"));
        vector<string> assignedRoles = assignRoles(playerCount);

        crow::json::wvalue jsonRoles;
        crow::json::wvalue::list roleList;
        for (const auto& role : assignedRoles) {
            roleList.emplace_back(role);
        }
        jsonRoles["roles"] = std::move(roleList);

        return crow::response(jsonRoles);
    });

    CROW_ROUTE(app, "/")([]() { return crow::response(loadFile("src/index.html")); });
    CROW_ROUTE(app, "/script.js")([]() { return crow::response(loadFile("src/script.js")); });

    app.port(18080).multithreaded().run();
}

