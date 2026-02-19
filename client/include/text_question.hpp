#ifndef TEXT_QUESTION_HPP_
#define TEXT_QUESTION_HPP_

#include <nlohmann/json_fwd.hpp>
// #include <optional>
#include "abstract_question.hpp"
#include <QWidget>

class QLabel;
class QLineEdit;

namespace survey {
class TextBlock : public QuestionBlock {
    Q_OBJECT
public:
    TextBlock(
        const nlohmann::json &block,
        // std::optional<std::string> correct_answer,
        QWidget *parent = nullptr
    );

    void save_answer(nlohmann::json &) const override;

private:
    QLabel* question_;
    QLineEdit* answer_;
    // std::optional<std::string> correct_answer_;
};
}  // namespace survey

#endif  // TEXT_QUESTION_HPP_

