#ifndef QUESTION_BLOCK_HPP_
#define QUESTION_BLOCK_HPP_
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace survey {
class QuestionBlock {
public:
    QuestionBlock() = default;
    virtual ~QuestionBlock() = default;
    QuestionBlock(const QuestionBlock &) = delete;
    QuestionBlock(QuestionBlock &&) = delete;
    QuestionBlock &operator=(const QuestionBlock &) = delete;
    QuestionBlock &operator=(QuestionBlock &&) = delete;

    virtual void print() const = 0;
    virtual bool parse_input(const std::string &) = 0;
    virtual void save_result(nlohmann::json &)
        const = 0;  // saves in json following sm rules
};
}  // namespace survey

#endif  // QUESTION_BLOCK_HPP_