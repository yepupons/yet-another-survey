#include "single_choice_question.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace survey {
void SingleChoiceBlock::print() const {
    std::cout << question_ << '\n';
    for (int counter = 0; const auto &option : options_) {
        std::cout << ++counter << ". " << option << '\n';
    }
    std::cout << "Input your choice here: ";
}

bool SingleChoiceBlock::parse_input(const std::string &input) {
    std::stringstream ss(input);
    if (int answer; ss >> answer && ss.str().empty()) {
        answer_ = answer;
        return true;
    }
    return false;
}

void SingleChoiceBlock::save_result(nlohmann::json &answers_data) const {
    answers_data["answers"].push_back(answer_);
}
}  // namespace survey