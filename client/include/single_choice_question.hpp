#ifndef SINGLE_CHOICE_QUESTION_HPP_
#define SINGLE_CHOICE_QUESTION_HPP_

#include <nlohmann/json_fwd.hpp>
// #include <optional>
#include <QWidget>
#include "abstract_question.hpp"

class QLabel;
class QButtonGroup;

namespace survey {
class SingleChoiceBlock : public QuestionBlock {
    Q_OBJECT
public:
    SingleChoiceBlock(
        const nlohmann::json &block,
        // std::optional<int> correct_answer,
        QWidget *parent = nullptr
    );

    void save_answer(nlohmann::json &) const override;
    bool has_answer() const override;

private slots:
    // void onButtonClicked();

private:
    QLabel *question_;
    QButtonGroup *options_;
    // std::optional<int> correct_answer_;
};
}  // namespace survey

#endif  // SINGLE_CHOICE_QUESTION_HPP_
