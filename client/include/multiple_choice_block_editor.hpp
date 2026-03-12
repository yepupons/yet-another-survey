#ifndef MULTIPLE_CHOICE_BLOCK_EDITOR_HPP_
#define MULTIPLE_CHOICE_BLOCK_EDITOR_HPP_
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <nlohmann/json.hpp>
#include <vector>
#include "block_editor.hpp"

namespace survey {
class MultipleChoiceBlockEditor : public BlockEditor {
    Q_OBJECT
public:
    explicit MultipleChoiceBlockEditor(QWidget *parent = nullptr);

    bool is_saved() const override {
        return saved_;
    }

    nlohmann::json to_json() const override;

private slots:
    void on_add_option();
    void on_save();

private:
    QLineEdit *question_ = nullptr;
    QVBoxLayout *optionsLayout_ = nullptr;
    QPushButton *addOption_ = nullptr;
    QPushButton *save_ = nullptr;
    QCheckBox *required_ = nullptr;

    std::vector<QLineEdit *> optionEdits_;
    bool saved_ = false;
};
}  // namespace survey

#endif