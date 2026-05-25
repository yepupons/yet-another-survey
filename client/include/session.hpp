#ifndef SESSION_HPP_
#define SESSION_HPP_

#include <string>

namespace survey {
class Session {
public:
    Session();

    std::string get_id() const;
    std::string get_access_token() const;
    bool is_authenticated() const;
    void set_auth(const std::string &id, const std::string &access_token);
    void clear_auth();

private:
    std::string id_;
    std::string access_token_;
};

Session &session();
}  // namespace survey

#endif
