#ifndef BLOCK_EDITOR_HPP_
#define BLOCK_EDITOR_HPP_
#include <QWidget>
#include <nlohmann/json.hpp>

namespace survey {
class BlockEditor : public QWidget {
public:
    BlockEditor(QWidget *parent) : QWidget(parent) {
    }

    virtual ~BlockEditor() = default;
    virtual nlohmann::json to_json() const = 0;
    virtual bool is_saved() const = 0;
};
}  // namespace survey

#endif
