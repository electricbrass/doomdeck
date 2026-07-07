export module errors;

import std;

// TODO: add stack trace
class ApplicationError : public std::runtime_error {
public:
    ApplicationError(const std::string& message) : std::runtime_error(message) {}
};
