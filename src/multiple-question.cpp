#include "multiple-question.hpp"
#include <cctype>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

namespace survey {

void MultipleQuestionBlock::print() const {
    std::cout << '\n';
    for (int i = 0; i < 10; i++) {std::cout << '-';}
    std::cout << '\n';
    std::cout << question_ << "\nChoose one or more answers\n";
    for (int counter = 0, j = 0; const auto &option : options_) {
        std::cout << ++counter << ". " << option;
        if (!answer_.empty()){
            if (j < answer_.size() && counter == answer_[j] && find(correct_answer_.begin(), correct_answer_.end(), answer_[j]) != correct_answer_.end()) {
                std::cout << " +";
                ++j;
            } else if (j < answer_.size() && counter == answer_[j] && find(correct_answer_.begin(), correct_answer_.end(), answer_[j]) == correct_answer_.end()){
                std::cout << " -";
                ++j;
            } else if (find(correct_answer_.begin(), correct_answer_.end(), counter) != correct_answer_.end()){
                std::cout << " <-";
            }
        }
        std::cout << '\n';
      }
    if (answer_.empty()) {
        std::cout << "Input your choice here: ";
    }
}

bool MultipleQuestionBlock::parse_input(const std::string &answer) {
    int n = answer.size();
    for (int i = 0; i < n;) {
        if (isdigit(answer[i])) {
            int x = 0;
            while (i < n && isdigit(answer[i])) {
                x = x * 10 + (answer[i] - '0');
                ++i;
            }
            if (x <= options_.size() && x > 0) {
                answer_.push_back(x);
            } else {
                std::cout << "Incorrect input!\n";
                return false;
            }
        } else {
            if (answer[i] == ',' || answer[i] == ' ') {
                ++i;
            } else {
                std::cout << "Incorrect input!\n";
                return false;
            }
        }
    }
    std::sort(answer_.begin(), answer_.end());
    return true;
}

void MultipleQuestionBlock::save_result(nlohmann::json &answers_data) const {
    answers_data["answers"].push_back(answer_);
}
}  // namespace survey
