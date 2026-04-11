#include "created_surveys_window.hpp"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include "pretty_view.hpp"
#include "server_interaction.hpp"
#include "session.hpp"

namespace survey {
CreatedSurveysWindow::CreatedSurveysWindow(QWidget *parent) : QDialog(parent) {
    auto *layout = new QVBoxLayout();
    nlohmann::json surveys_ids;
    try {
        surveys_ids =
            ServerInteraction::get_created_surveys(session().get_id());
    } catch (const std::exception &e) {
        show_message_box(this, QMessageBox::Warning, "Error", e.what());
        return;
    }
    if (surveys_ids.empty()) {
        layout->addWidget(new QLabel("No created surveys yet.", this));
        setLayout(layout);
        return;
    }

    for (int id : surveys_ids) {
        auto *row = new QHBoxLayout();
        row->addWidget(new QLabel(QString::number(id), this));

        auto *import_button = new QPushButton("Import statistics", this);
        row->addWidget(import_button);
        connect(import_button, &QPushButton::clicked, this, [this, id]() {
            import_statistics(id);
        });

        layout->addLayout(row);
    }

    setLayout(layout);
}

void CreatedSurveysWindow::import_statistics(int survey_id) {
    std::ofstream file(std::to_string(survey_id) + ".txt");
    if (!file.is_open()) {
        show_message_box(
            this, QMessageBox::Warning, "Error", "Unable to open file"
        );
        return;
    }
    nlohmann::json survey = ServerInteraction::get_survey(survey_id);
    nlohmann::json stats = ServerInteraction::get_survey_statistics(survey_id);
    file << "Total number of answers: " << stats["total_answers"] << "\n\n";
    file << "Statistic by sections:\n\n";
    for (int i = 0; i < survey["sections"].size(); ++i) {
        file << "Section " << survey["sections"][i]["title"] << ":\n\n";
        for (int j = 0; j < survey["sections"][i]["questions"].size(); ++j) {
            const auto &question = survey["sections"][i]["questions"][j];
            file << "Question " << question["text"] << ":\n";
            if (question["type"] == "single" ||
                question["type"] == "multiple") {
                for (int k = 1; k <= question["options"].size(); ++k) {
                    file << question["options"][k - 1] << ": ";
                    if (stats["sections"][i][j].contains(std::to_string(k))) {
                        file << stats["sections"][i][j][std::to_string(k)];
                    } else {
                        file << 0;
                    }
                    file << " answer(s)\n";
                }
            } else if (question["type"] == "text") {
                for (const auto &answer_count :
                     stats["sections"][i][j].items()) {
                    file << '\"' << answer_count.key()
                         << "\": " << answer_count.value() << " answer(s)\n";
                }
            }
            file << '\n';
        }
    }
    file.close();
}
}  // namespace survey