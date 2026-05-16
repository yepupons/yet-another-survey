#include "multiple_choice_block.hpp"
#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QList>
#include <QString>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "server_interaction.hpp"

namespace survey {
MultipleChoiceBlock::MultipleChoiceBlock(
    const nlohmann::json &block,
    QWidget *parent
)
    : Block(parent, block.value("required", false)) {
    setObjectName("questionBlockContainer");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);


    QLabel *image = nullptr;
    QPixmap pixmap;

    if (block.contains("image_path") && block.at("image_path").is_string()) {
        const QString image_path =
            QString::fromStdString(block.at("image_path").get<std::string>());
        pixmap.load(image_path);
    } else if (block.contains("image") && block.at("image").is_string()) {
        try {
            const std::string image_data =
                ServerInteraction::get_image(block.at("image").get<std::string>());

            const QByteArray byte_array = QByteArray::fromStdString(image_data);
            pixmap.loadFromData(byte_array);
        } catch (const std::exception &) {
            pixmap = QPixmap();
        }
    }

    if (!pixmap.isNull()) {
        image = new QLabel(this);
        image->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        image->setAlignment(Qt::AlignCenter);
        image->setPixmap(pixmap.scaled(
            700, 700, Qt::KeepAspectRatio, Qt::SmoothTransformation
        ));
    }

    QString question_text = QString::fromStdString(block.at("text"));
    QString html = question_text.toHtmlEscaped();

    if (block.value("required", false)) {
        html += " <span style='color: red; font-weight: 400; '>*</span>";
    }

    question_ = new QLabel(html, this);
    question_->setTextFormat(Qt::RichText);
    question_->setObjectName("questionTitle");
    question_->setWordWrap(true);
    question_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    options_ = new QButtonGroup(this);
    options_->setExclusive(false);

    auto *outer_layout = new QVBoxLayout(this);
    outer_layout->setContentsMargins(0, 0, 0, 0);
    outer_layout->setSpacing(0);

    auto *row_layout = new QHBoxLayout();
    row_layout->setContentsMargins(0, 0, 0, 0);
    row_layout->setSpacing(0);

    row_layout->addStretch();

    auto *content = new QWidget(this);
    content->setObjectName("questionContentWrapper");
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    content->setFixedWidth(720);

    auto *content_layout = new QVBoxLayout(content);
    content_layout->setContentsMargins(0, 0, 0, 0);
    content_layout->setSpacing(0);

    auto *card = new QWidget(content);
    card->setObjectName("questionCard");
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *card_layout = new QVBoxLayout(card);
    card_layout->setContentsMargins(24, 24, 24, 24);
    card_layout->setSpacing(14);
    card_layout->addWidget(question_);
    if (image != nullptr) {
        card_layout->addWidget(image);
    }

    int id = 0;
    for (const std::string &option : block.at("options")) {
        auto *button = new QCheckBox(QString::fromStdString(option), card);
        options_->addButton(button, ++id);
        button->setObjectName("choiceOption");
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        card_layout->addWidget(button);
    }

    content_layout->addWidget(card);
    row_layout->addWidget(content);
    row_layout->addStretch();

    outer_layout->addLayout(row_layout);
}

void MultipleChoiceBlock::save_answer(nlohmann::json &answer_data) const {
    QList<int> answer;
    for (const auto &option : options_->buttons()) {
        if (option->isChecked()) {
            answer.push_back(options_->id(option));
        }
    }
    answer_data.push_back({{"answer", answer}});
}

bool MultipleChoiceBlock::has_answer() const {
    return options_->checkedId() != -1;
}

void MultipleChoiceBlock::set_answer(const nlohmann::json &answer) {
    for (const auto &value : answer) {
        auto *button = options_->button(value.get<int>());
        if (button) {
            button->setChecked(true);
        }
    }
}

void MultipleChoiceBlock::set_read_only(bool read_only) {
    for (auto *button : options_->buttons()) {
        button->setEnabled(!read_only);
    }
}
}  // namespace survey
