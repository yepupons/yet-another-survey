#ifndef SINGLE_CHOICE_BLOCK_HPP_
#define SINGLE_CHOICE_BLOCK_HPP_

#include <nlohmann/json_fwd.hpp>
// #include <optional>
#include <QWidget>
#include "abstract_block.hpp"

class QLabel;
class QButtonGroup;

namespace survey {
class SingleChoiceBlock : public Block {
    Q_OBJECT
public:
    SingleChoiceBlock(
        const nlohmann::json &block,
        // std::optional<int> correct_answer,
        QWidget *parent = nullptr
    );

    void save_answer(nlohmann::json &) const override;
    bool has_answer() const override;
    std::optional<int> next_section() const override;

private:
    QLabel *question_;
    QButtonGroup *options_;
    // std::optional<int> correct_answer_;
    std::unordered_map<int, int> links_;
};
}  // namespace survey

#endif  // SINGLE_CHOICE_BLOCK_HPP_
