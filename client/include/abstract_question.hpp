#ifndef QUESTION_BLOCK_HPP_
#define QUESTION_BLOCK_HPP_

#include <nlohmann/json_fwd.hpp>
#include <QWidget>

namespace survey {
class QuestionBlock : public QWidget {
    Q_OBJECT
public:
    explicit QuestionBlock(QWidget *parent = nullptr) : QWidget(parent) {}

    virtual void save_answer(nlohmann::json &) const = 0;
};
}  // namespace survey

#endif  // QUESTION_BLOCK_HPP_