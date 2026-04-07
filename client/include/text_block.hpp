#ifndef TEXT_BLOCK_HPP_
#define TEXT_BLOCK_HPP_

#include <nlohmann/json_fwd.hpp>
// #include <optional>
#include <QWidget>
#include "abstract_block.hpp"

class QLabel;
class QLineEdit;

namespace survey {
class TextBlock : public Block {
    Q_OBJECT
public:
    TextBlock(
        const nlohmann::json &block,
        // std::optional<std::string> correct_answer,
        QWidget *parent = nullptr
    );

    void save_answer(nlohmann::json &) const override;
    bool has_answer() const override;

private:
    QLabel *question_;
    QLineEdit *answer_;
    // std::optional<std::string> correct_answer_;
};
}  // namespace survey

#endif  // TEXT_BLOCK_HPP_
