#ifndef TEXT_QUESTION_BLOCK_EDITOR_HPP_
#define TEXT_QUESTION_BLOCK_EDITOR_HPP_
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <QLabel>
#include <nlohmann/json.hpp>
#include "block_editor.hpp"

namespace survey {
class TextBlockEditor : public BlockEditor {
    Q_OBJECT
public:
    explicit TextBlockEditor(BuilderMode mode, QWidget *parent = nullptr);

    bool is_saved() const override {
        return saved_;
    }

    nlohmann::json to_json() const override;

private slots:
    void on_save();

private:
    QTextEdit *text_ = nullptr;
    QPushButton *save_ = nullptr;
    QCheckBox *required_ = nullptr;
    QLabel *correctAnswersLabel_ = nullptr;
    QTextEdit *correctAnswers_ = nullptr;


    bool saved_ = false;
};
}  // namespace survey

#endif