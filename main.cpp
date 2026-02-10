#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <abstract_question.hpp>
#include <memory>
#include <string>
#include <unordered_map>

// TODO: make it everywhere
using json = nlohmann::json;
// TODO: maybe change location of enum?
enum class BlockType {Text, Multiple, OneOption};

const std::unordered_map<std::string, BlockType> COMPARATOR {{"text", BlockType::Text}, \
                                                        {"multiple", BlockType::Multiple}, \
                                                        {"one-option", BlockType::OneOption} };



int main(){
    std::ifstream f("test_survey.json");
    json survey_data = json::parse(f);
    std::vector<std::unique_ptr<survey::QuestionBlock>> question_list;

    for (auto question_block : survey_data["questions"]){
        std::string question_type = question_block["question_type"];
        switch (COMPARATOR.at(question_type)) {
            case BlockType::Text:{
                // question_list.push_back(std::unique_ptr<survey::QuestionBlock>(new survey::QuestionBlock()));
                break;
            }
            case BlockType::Multiple:{
                // question_list.push_back(std::unique_ptr<survey::QuestionBlock>(new survey::QuestionBlock()));
                break;
            }
            case BlockType::OneOption:{
                // question_list.push_back(std::unique_ptr<survey::QuestionBlock>(new survey::QuestionBlock()));
                break;
            }
        }
    }

}