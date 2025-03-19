#include <crow.h>
#include <vector>
#include <random>
#include <fstream>

using namespace std;

vector<string> roles = {"Dân làng", "Sói", "Bảo vệ", "Tiên tri", "Phù thủy"};

string getRandomRole() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dis(0, roles.size() - 1);
    return roles[dis(gen)];
}

// Hàm đọc file HTML
string loadFile(const string& filename) {
    ifstream file(filename); // Đọc file theo tham số truyền vào
    if (!file) return "Lỗi: Không tìm thấy file: " + filename;
    return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}


int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/random-role")([](){
        return crow::response(getRandomRole());
    });

    CROW_ROUTE(app, "/")([](){
        return crow::response(loadFile("src/index.html"));
    });

    CROW_ROUTE(app, "/script.js")([](){
        return crow::response(loadFile("src/script.js"));
    });

    app.port(18080).multithreaded().run();
}
