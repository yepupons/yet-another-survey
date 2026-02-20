#ifndef QUESTION_BLOCK_HPP_
#define QUESTION_BLOCK_HPP_

#include <QWidget>
#include <nlohmann/json_fwd.hpp>

namespace survey {
class QuestionBlock : public QWidget {
    Q_OBJECT
public:
    explicit QuestionBlock(QWidget *parent = nullptr, bool required = false)
        : QWidget(parent), required_(required) {
    }

    virtual void save_answer(nlohmann::json &) const = 0;
    virtual bool has_answer() const = 0;

    bool is_required() const {
        return required_;
    }

    bool is_valid() const {
        return !is_required() || has_answer();
    }

private:
    bool required_;
};
}  // namespace survey

#endif  // QUESTION_BLOCK_HPP_