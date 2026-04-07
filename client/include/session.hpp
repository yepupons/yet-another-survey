#ifndef SESSION_HPP_
#define SESSION_HPP_

namespace survey {
class Session {
    int id_;

public:
    Session();
    [[nodiscard]] int get_id() const;
    void set_id(int id);
};

Session &session();
}  // namespace survey

#endif