#ifndef QUIZ_CHOICE_BLOCK_HPP_
#define QUIZ_CHOICE_BLOCK_HPP_

#include <QWidget>
#include <nlohmann/json_fwd.hpp>
#include "abstract_block.hpp"

class QLabel;
class QButtonGroup;

namespace survey {
class QuizChoiceBlock : public Block {
    Q_OBJECT
public:
    QuizChoiceBlock(const nlohmann::json &block, QWidget *parent = nullptr);

    void save_answer(nlohmann::json &) const override;
    bool has_answer() const override;
    std::optional<int> next_section() const override;
    void set_answer(const nlohmann::json &answer) override;
    void set_read_only(bool read_only) override;

private:
    QLabel *question_;
    QButtonGroup *options_;
    std::unordered_map<int, int> links_;
};
}  // namespace survey

#endif
