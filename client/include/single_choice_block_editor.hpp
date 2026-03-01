#ifndef SINGLE_CHOICE_BLOCK_EDITOR_HPP_
#define SINGLE_CHOICE_BLOCK_EDITOR_HPP_
#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <vector>
#include <nlohmann/json.hpp>
#include "block_editor.hpp"

namespace survey {
class SingleChoiceBlockEditor : public BlockEditor {
    Q_OBJECT
public:
    explicit SingleChoiceBlockEditor(QWidget *parent = nullptr);

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

    std::vector<QLineEdit*> optionEdits_;
    bool saved_ = false;
};
}

#endif