#pragma once
#include <string>

struct ParseError {
    std::string message;
    size_t offset;
};

template<typename T>
using ParseResult = std::variant<T, ParseError>;