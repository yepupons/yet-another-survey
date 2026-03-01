#include "survey_builder_window.hpp"
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QLabel>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <random>
#include "single_choice_block_editor.hpp"
#include "multiple_choice_block_editor.hpp"
#include "text_question_block_editor.hpp"

namespace survey {

SurveyBuilderWindow::SurveyBuilderWindow(Mode mode, QWidget *parent)
    : QWidget(parent), mode_(mode) {

    setWindowTitle(mode_ == Mode::Survey ? "Survey Builder (Survey)" : "Survey Builder (Test)");
    resize(800, 600);

    auto *root = new QVBoxLayout(this);

    auto *title = new QLabel("Survey constructor", this);
    root->addWidget(title);

    scroll_ = new QScrollArea(this);
    scroll_->setWidgetResizable(true);

    content_ = new QWidget(scroll_);
    contentLayout_ = new QVBoxLayout(content_);
    contentLayout_->setAlignment(Qt::AlignTop);

    scroll_->setWidget(content_);
    root->addWidget(scroll_, 1);

    auto *bottomRow = new QHBoxLayout();
    bottomRow->addStretch();

    addBlockButton_ = new QPushButton("+", this);
    addBlockButton_->setFixedSize(40, 40);
    bottomRow->addWidget(addBlockButton_);

    root->addLayout(bottomRow);

    saveSurveyButton_ = new QPushButton("Save survey", this);
    root->addWidget(saveSurveyButton_);

    connect(addBlockButton_, &QPushButton::clicked, this, &SurveyBuilderWindow::add_block_menu);
    connect(saveSurveyButton_, &QPushButton::clicked, this, &SurveyBuilderWindow::save_survey);
}

void SurveyBuilderWindow::add_block_menu() {
    auto *menu = new QMenu(this);
    auto *single = menu->addAction("Single choice");
    auto *multiple  = menu->addAction("Multiple choice");
    auto *text   = menu->addAction("Text block");

    QAction *chosen = menu->exec(addBlockButton_->mapToGlobal(QPoint(0, addBlockButton_->height())));
    if (!chosen) return;

    if (chosen == single) add_single_choice();
    else if (chosen == multiple) add_multiple_choice();
    else if (chosen == text) add_text_block();
}

void SurveyBuilderWindow::add_single_choice() {
    auto *w = new SingleChoiceBlockEditor(content_);
    contentLayout_->addWidget(w);
}

void SurveyBuilderWindow::add_multiple_choice() {
    auto *w = new MultipleChoiceBlockEditor(content_);
    contentLayout_->addWidget(w);
}

void SurveyBuilderWindow::add_text_block() {
    auto *w = new TextBlockEditor(content_);
    contentLayout_->addWidget(w);
}

nlohmann::json SurveyBuilderWindow::build_survey_json(int id) const {
    nlohmann::json j;
    j["survey_data"] = {
        {"id", id},
        {"type", "survey"}
    };
    j["questions"] = nlohmann::json::array();

    for (int i = 0; i < contentLayout_->count(); ++i) {
        QWidget *w = contentLayout_->itemAt(i)->widget();
        if (!w) continue;

        auto *base = dynamic_cast<BlockEditor*>(w);
        if (!base) continue;

        if (!base->is_saved()) continue;

        j["questions"].push_back(base->to_json());
    }
    return j;
}

std::string SurveyBuilderWindow::generate_survey_id() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::ostringstream oss;
    oss << ms;
    return oss.str();
}

bool SurveyBuilderWindow::save_survey_to_disk(const std::string &id,
                                          const nlohmann::json &j,
                                          std::string &err) {
    std::filesystem::path root = std::filesystem::current_path() / "public";
    if (!std::filesystem::exists(root)) {
        std::filesystem::path root2 = std::filesystem::current_path() / ".." / "public";
        if (std::filesystem::exists(root2)) root = root2;
    }

    if (!std::filesystem::exists(root)) {
        err = "public directory not found (./public or ../public)";
        return false;
    }

    std::filesystem::path surveyDir = root / id / "survey";
    std::error_code ec;
    std::filesystem::create_directories(surveyDir, ec);
    if (ec) {
        err = "Failed to create directories: " + ec.message();
        return false;
    }

    std::filesystem::path filePath = surveyDir / "data.json";
    std::ofstream out(filePath, std::ios::binary);
    if (!out) {
        err = "Failed to open file for writing: " + filePath.string();
        return false;
    }

    out << j.dump(2) << "\n";
    out.flush();
    if (!out.good()) {
        err = "Write failed: " + filePath.string();
        return false;
    }
    return true;
}

void SurveyBuilderWindow::save_survey() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(100000, 999999);

    nlohmann::json j = build_survey_json(dist(gen));

    if (!j.contains("questions") || j["questions"].empty()) {
        QMessageBox::warning(this, "Error", "There are no saved blocks. First, add and save at least one block.");
        return;
    }

    std::string id = generate_survey_id();

    std::string err;
    if (!save_survey_to_disk(id, j, err)) {
        QMessageBox::warning(this, "Error", QString::fromStdString(err));
        return;
    }

    QMessageBox::information(
        this,
        "Saved",
        "Survey has benn saved.\nYour ID:\n" + QString::fromStdString(id)
    );
}
}