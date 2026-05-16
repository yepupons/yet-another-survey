#include "session.hpp"
#include <cstdlib>
#include <fstream>
#include <random>
#include <string>

namespace survey {

static std::string session_file_path() {
    const char *home = std::getenv("HOME");
    return std::string(home ? home : ".") + "/.yas_session";
}

static std::string generate_uuid() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> hex(0, 15);
    std::uniform_int_distribution<int> variant(8, 11);

    const char *digits = "0123456789abcdef";
    std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    for (auto &c : uuid) {
        if (c == 'x') c = digits[hex(rng)];
        else if (c == 'y') c = digits[variant(rng)];
    }
    return uuid;
}

Session::Session() {
    std::ifstream file(session_file_path());
    if (std::getline(file, id_) && !id_.empty()) return;

    id_ = generate_uuid();
    std::ofstream out(session_file_path());
    out << id_;
}

std::string Session::get_id() const {
    return id_;
}

void Session::set_id(const std::string &id) {
    id_ = id;
    std::ofstream out(session_file_path());
    out << id_;
}

Session &session() {
    static Session session;
    return session;
}
}  // namespace survey