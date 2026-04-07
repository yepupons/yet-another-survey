#include "session.hpp"
#include <chrono>

namespace survey {
Session::Session() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()
    )
                  .count();
    id_ = static_cast<int>(ms % 1000000);
}

int Session::get_id() const {
    return id_;
}

void Session::set_id(int id) {
    id_ = id;
}

Session &session() {
    static Session session;
    return session;
}
}  // namespace survey