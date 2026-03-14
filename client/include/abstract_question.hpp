#ifndef QUESTION_BLOCK_HPP_
#define QUESTION_BLOCK_HPP_

#include <QWidget>
#include <nlohmann/json_fwd.hpp>
#include <optional>

namespace survey {
class QuestionBlock : public QWidget {
    Q_OBJECT
public:
    explicit QuestionBlock(QWidget *parent = nullptr, bool required = false)
        : QWidget(parent), required_(required) {
    }

    virtual void save_answer(nlohmann::json &) const = 0;
    virtual bool has_answer() const = 0;

    virtual std::optional<int> next_section() const {
        return std::nullopt;
    }

    bool is_valid() const {
        return !required_ || has_answer();
    }

private:
    bool required_;
};
}  // namespace survey

#endif  // QUESTION_BLOCK_HPP_