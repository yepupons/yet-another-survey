#ifndef SESSION_HPP_
#define SESSION_HPP_

#include <string>

namespace survey {
class Session {
    std::string id_;

public:
    Session();
    [[nodiscard]] std::string get_id() const;
    void set_id(const std::string &id);
};

Session &session();
}  // namespace survey

#endif