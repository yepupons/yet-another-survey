#ifndef MULTIPLE_CHOICE_BLOCK_EDITOR_HPP_
#define MULTIPLE_CHOICE_BLOCK_EDITOR_HPP_
#include <QButtonGroup>
#include <QCheckBox>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <nlohmann/json.hpp>
#include "abstract_block_editor.hpp"

namespace survey {
class MultipleChoiceBlockEditor : public BlockEditor {
    Q_OBJECT
public:
    explicit MultipleChoiceBlockEditor(Created_Type type, QWidget *parent = nullptr);

    void to_json(
        bool preview_mode,
        std::function<void(const nlohmann::json &)> callback
    ) const override;

private slots:
    void add_option();

private:
    QList<QLineEdit *> options_;
    QVBoxLayout *options_layout_ = nullptr;
    QPushButton *add_option_button_ = nullptr;
    QCheckBox *required_ = nullptr;

    QButtonGroup *correct_answers_ = nullptr;
};
}  // namespace survey

#endif