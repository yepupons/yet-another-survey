#ifndef MULTIPLE_CHOICE_QUESTION_HPP_
#define MULTIPLE_CHOICE_QUESTION_HPP_

#include <nlohmann/json_fwd.hpp>
// #include <optional>
#include <QWidget>
#include "abstract_question.hpp"

class QLabel;
class QButtonGroup;

namespace survey {
class MultipleChoiceBlock : public QuestionBlock {
    Q_OBJECT
public:
    MultipleChoiceBlock(
        const nlohmann::json &block,
        // std::optional<std::vector<int>> correct_answer,
        QWidget *parent = nullptr
    );

    void save_answer(nlohmann::json &) const override;
    bool has_answer() const override;

private slots:
    // void onButtonClicked();

private:
    QLabel *question_;
    QButtonGroup *options_;
    // std::optional<std::vector<int>> correct_answer_;
};
}  // namespace survey

#endif  // MULTIPLE_CHOICE_QUESTION_HPP_
