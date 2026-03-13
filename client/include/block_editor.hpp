#ifndef BLOCK_EDITOR_HPP_
#define BLOCK_EDITOR_HPP_
#include <QWidget>
#include "builder_mode.hpp"
#include <nlohmann/json.hpp>

namespace survey {
class BlockEditor : public QWidget {
public:
    BlockEditor(BuilderMode mode, QWidget *parent) : mode_(mode), QWidget(parent) {
    }

    virtual ~BlockEditor() = default;
    virtual nlohmann::json to_json() const = 0;
    virtual bool is_saved() const = 0;

protected:
    bool is_test_mode() const {
        return mode_ == BuilderMode::Test;
    }

    BuilderMode mode() const {
        return mode_;
    }

private:
    BuilderMode mode_;
};
}  // namespace survey

#endif
