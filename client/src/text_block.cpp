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

    if (block.contains("image_path") && block.at("image_path").is_string()) {
        const QString image_path =
            QString::fromStdString(block.at("image_path").get<std::string>());
        QPixmap pixmap;
        if (pixmap.load(image_path)) {  // NOT WORK
            QLabel *image = new QLabel(this);
            image->setSizePolicy(
                QSizePolicy::Expanding, QSizePolicy::Preferred
            );
            image->setAlignment(Qt::AlignCenter);
            image->setPixmap(pixmap.scaled(
                500, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation
            ));
            card_layout->addWidget(image);
        }
    } else if (block.contains("image") && block.at("image").is_string()) {
        server().get_image(
            block.at("image").get<std::string>(),
            [=, this](const std::string &image_data) {
                const QByteArray byte_array =
                    QByteArray::fromStdString(image_data);
                QPixmap pixmap;
                if (pixmap.loadFromData(byte_array)) {
                    QLabel *image = new QLabel(this);
                    image->setSizePolicy(
                        QSizePolicy::Expanding, QSizePolicy::Preferred
                    );
                    image->setAlignment(Qt::AlignCenter);
                    image->setPixmap(pixmap.scaled(
                        500, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation
                    ));
                    card_layout->addWidget(image);
                }
            },
            [=, this](const std::string &) {}
        );
    }

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
