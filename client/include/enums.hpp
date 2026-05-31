#ifndef ENUMS_HPP_
#define ENUMS_HPP_

#include <string>
#include <unordered_map>

namespace survey {
enum class BlockType { Text, Multiple, Single };

const std::unordered_map<std::string, BlockType> BLOCK_TYPE{
    {"text", BlockType::Text},
    {"multiple", BlockType::Multiple},
    {"single", BlockType::Single}
};

enum class SurveyType { Survey, Test, Quiz };

const std::unordered_map<std::string, SurveyType> SURVEY_TYPE{
    {"survey", SurveyType::Survey},
    {"test", SurveyType::Test},
    {"quiz", SurveyType::Quiz}
};
}

#endif // ENUMS_HPP_