#ifndef UTILITY_H
#define UTILITY_H
 
#include <stdexcept>
#include <string>

class Exception : public std::exception
{
private:
    std::string message;
    std::exception ex;
public:
    enum class ErrorCode {
        UNKNOWN_ERROR = -1,
        SUCCESS = 0,
        INVALID_TYPE = 1,
        UNEXPECTED_NULLPTR = 2,
        UNEXPECTED_CHAR = 3,
        INDEX_OUT_OF_BOUNDS = 4,
        EMPTY_STRUCTURE = 5,
        NEGATIVE_SIZE_DIFFERENCE = 6,
        INVALID_SELECTION = 7,
        INVALID_INPUT = 8,
        INVALID_SIZE = 9,
        EMPTY_OPTION = 10,
        NULL_DEREFERENCE = 11,
        NULL_DEPENDENT_JUMP = 12,
        KEY_COLLISION = 13,
        INVALID_ITERATOR = 14,
        ABSENT_KEY = 15,
        ERROR_CREATING_FILE = 16,
        RELATIVE_PHYSICAL_PATH = 17,
        CYCLIC_MOVE = 18, 
        CONCAT_WITH_ABS_PATH = 19,
        FORK_FAILURE = 20,
        EXEC_FAILURE = 21,
        WAITPID_FAILURE = 22
    };
public:
    explicit Exception( std::exception& ex ) : ex(ex) {
        this->ex = ex;
        try {
            throw ex;
        } catch( std::bad_alloc& ex ) {
            this->message = "Error. Unable to allocate memory.";
        } catch( std::invalid_argument& ex ) {
            this->message = "Error. Invalid argument.";
        } catch( ... ) {
            this->message = "Unknown error.";
        }
    }
    explicit Exception( ErrorCode code ) {
        this->ex = std::exception();
        switch ( code ) {
        case ErrorCode::SUCCESS:
            this->message = "Success!";
            break;
        case ErrorCode::UNEXPECTED_NULLPTR:
            this->message = "Error. Provided data contains null-pointer.";
            break;
        case ErrorCode::UNEXPECTED_CHAR:
            this->message = "Error. Invalid input, expected digits.";
            break;
        case ErrorCode::INDEX_OUT_OF_BOUNDS:
            this->message = "Error. Index out of bounds.";
            break;
        case ErrorCode::EMPTY_STRUCTURE:
            this->message = "Error. Unable to process empty data structure.";
            break;
        case ErrorCode::NEGATIVE_SIZE_DIFFERENCE:
            this->message = "Error. Size difference cannot be negative.";
            break;
        case ErrorCode::INVALID_TYPE:
            this->message = "Error. Invalid type.";
            break;
        case ErrorCode::INVALID_SELECTION:
            this->message = "Error. Make sure you've correctly selected an item.";
            break;
        case ErrorCode::INVALID_INPUT:
            this->message = "Error. Make sure you've correctly provided all neccessary input.";
            break;
        case ErrorCode::INVALID_SIZE:
            this->message = "Error. Invalid size.";
            break;
        case ErrorCode::EMPTY_OPTION:
            this->message = "Error. Optional type is undefined.";
            break;
        case ErrorCode::NULL_DEREFERENCE:
            this->message = "Error. Attempted to dereference a null-pointer.";
            break;
        case ErrorCode::NULL_DEPENDENT_JUMP:
            this->message = "Error. Attempt of using pointer arithmetic using null-pointer.";
            break;
        case ErrorCode::KEY_COLLISION:
            this->message = "Error. Added key already exists.";
            break;
        case ErrorCode::INVALID_ITERATOR:
            this->message = "Error. Unable to create iterator with provided input.";
            break;
        case ErrorCode::ABSENT_KEY:
            this->message = "Error. Element with requested key is absent in the tree.";
            break;
        case ErrorCode::ERROR_CREATING_FILE:
            this->message = "Error. Unable to create a file.";
            break;
        case ErrorCode::RELATIVE_PHYSICAL_PATH:
            this->message = "Error. Physical path must be specified as absolute.";
            break;
        case ErrorCode::CYCLIC_MOVE:
            this->message = "Error. Attempt of moving directory into itself.";
            break;
        case ErrorCode::CONCAT_WITH_ABS_PATH:
            this->message = "Error. Unable to perform concatenation with second argument being absolute path.";
            break;
        case ErrorCode::FORK_FAILURE:
            this->message = "Error. fork() failed to create child process.";
            break;
        case ErrorCode::EXEC_FAILURE:
            this->message = "Error. exec() failed.";
            break;
        case ErrorCode::WAITPID_FAILURE:
            this->message = "Error. waitpid() failed.";
            break;
        case ErrorCode::UNKNOWN_ERROR:
            this->message = "Unknown error.";
            break;
        }
    }
    explicit Exception( const std::string& message ) {
        this->message = message;
        this->ex = std::exception();
    }
public: 
    const char* what() const noexcept override {
        return this->message.c_str();
    }
};

class ExitSignal : public std::exception
{
private:
    std::string message;
public:
    ExitSignal() {
        message = "Received exit signal from user.";
    }

    explicit ExitSignal( const std::string& msg ) {
        message = msg;
    }
public:
    const char* what() const noexcept override {
        return message.c_str();
    }
};

#endif // UTILITY_H