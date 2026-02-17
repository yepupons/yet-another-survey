#include "single_choice_question.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace survey {
void SingleChoiceBlock::print_question() const {
    std::cout << '\n';
    for (int i = 0; i < 10; i++) {
        std::cout << '-';
    }
    std::cout << '\n' << question_ << '\n';
    for (int counter = 0; const auto &option : options_) {
        std::cout << ++counter << ". " << option << '\n';
    }
    std::cout << "Input your choice here: ";
}

void SingleChoiceBlock::print_answer(const bool is_test) const {
    std::cout << '\n';
    for (int i = 0; i < 10; i++) {
        std::cout << '-';
    }
    std::cout << '\n';
    std::cout << question_ << '\n';
    for (int counter = 0; const auto &option : options_) {
        std::cout << ++counter << ". " << option;
        if (is_test) {
            if (counter == answer_ && answer_ == correct_answer_) {
                std::cout << " +";
            } else if (counter == answer_ && answer_ != correct_answer_) {
                std::cout << " -";
            } else if (counter == correct_answer_) {
                std::cout << " <-";
            }
        } else {
            if (counter == answer_) {
                std::cout << " +";
            }
        }
        std::cout << '\n';
    }
}

bool SingleChoiceBlock::parse_input(const std::string &input) {
    std::stringstream ss(input);
    if (int answer; ss >> answer && answer <= options_.size()) {
        answer_ = answer;
        return true;
    }
    std::cout << "Incorrect input!\n";
    return false;
}

void SingleChoiceBlock::save_result(nlohmann::json &answers_data) const {
    answers_data["answers"].push_back(answer_);
}
}  // namespace survey
