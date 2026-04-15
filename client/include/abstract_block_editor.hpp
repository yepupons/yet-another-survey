#ifndef ABSTRACT_BLOCK_EDITOR_HPP_
#define ABSTRACT_BLOCK_EDITOR_HPP_
#include <QWidget>
#include <nlohmann/json.hpp>

namespace survey {
class BlockEditor : public QWidget {
    Q_OBJECT

public:
    BlockEditor(bool is_test, QWidget *parent)
        : QWidget(parent), is_test_(is_test) {
    }

    virtual ~BlockEditor() = default;
    virtual nlohmann::json to_json() const = 0;

signals:
    void remove_requested(BlockEditor *editor);

protected:
    bool is_test_;
};
}  // namespace survey

#endif
