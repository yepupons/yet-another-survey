#ifndef BLOCK_EDITOR_HPP_
#define BLOCK_EDITOR_HPP_
#include <QWidget>
#include <nlohmann/json.hpp>

namespace survey {
class BlockEditor : public QWidget {
public:
    BlockEditor(bool is_test, QWidget *parent) : QWidget(parent), is_test_(is_test) {
    }

    virtual ~BlockEditor() = default;
    virtual nlohmann::json to_json() const = 0;

protected:
    bool is_test_;
};
}  // namespace survey

#endif
