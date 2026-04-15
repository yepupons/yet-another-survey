#include "text_block.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <nlohmann/json.hpp>
#include "server_interaction.hpp"

namespace survey {
TextBlock::TextBlock(
    const nlohmann::json &block,
    // std::optional<std::string> correct_answer,
    QWidget *parent
)
    : Block(parent, block.value("required", false)) {
    setObjectName("questionBlockContainer");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QLabel *image = nullptr;
    if (block.contains("image")) {
        image = new QLabel(this);
        image->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        std::string image_data = ServerInteraction::get_image(block.at("image").get<std::string>()); 
        QByteArray byte_array = QByteArray::fromStdString(image_data);
        QPixmap pixmap;
        if (pixmap.loadFromData(byte_array)) {
            image->setPixmap(pixmap.scaled(
                700, 700,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            ));
        }
    }

    question_ = new QLabel(QString::fromStdString(block.at("text")), this);
    question_->setObjectName("questionTitle");
    question_->setWordWrap(true);
    question_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    answer_ = new QLineEdit(this);
    answer_->setObjectName("textAnswerInput");
    answer_->setPlaceholderText("Input your answer here:");
    answer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

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
    card_layout->addWidget(answer_);
    card_layout->addWidget(image);

    content_layout->addWidget(card);
    row_layout->addWidget(content);
    row_layout->addStretch();

    outer_layout->addLayout(row_layout);
}

void TextBlock::save_answer(nlohmann::json &answer_data) const {
    answer_data.push_back({{"answer", answer_->text().trimmed().toStdString()}}
    );
}

bool TextBlock::has_answer() const {
    return !answer_->text().trimmed().isEmpty();
}

void TextBlock::set_answer(const nlohmann::json &answer) {
    answer_->setText(QString::fromStdString(answer.get<std::string>()));
    return;
}

void TextBlock::set_read_only(bool read_only) {
    answer_->setReadOnly(read_only);
    if (!read_only) {
        answer_->setEnabled(true);
    }
}
}  // namespace survey
