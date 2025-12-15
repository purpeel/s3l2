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
        INFINITE_CALCULATION = 13,
        TRANSFINITE_ARITHMETIC = 14, 
        DEMATERIALIZED_ACCESS = 15, 
        UNKNOWN_ORDINALITY = 16,
        INVALID_ITERATOR = 17, 
        ITERATOR_AT_INFINITY = 18
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
        case ErrorCode::INFINITE_CALCULATION:
            this->message = "Error. Attempt of calculating requires endless computing time.";
            break;
        case ErrorCode::TRANSFINITE_ARITHMETIC:
            this->message = "Error. Specific transfinite arithmetic operation is undefined on provided input.";
            break;
        case ErrorCode::DEMATERIALIZED_ACCESS:
            this->message = "Error. Attempt of accessing dematerialized part of Lazy Sequence.";
            break;
        case ErrorCode::UNKNOWN_ORDINALITY:
            this->message = "Error. Unable to resolve transfinite indexing for this sequence.";
            break;
        case ErrorCode::INVALID_ITERATOR:
            this->message = "Error. Unable to get valid iterator for this collection.";
            break;  
        case ErrorCode::ITERATOR_AT_INFINITY:
            this->message = "Error. Getting such iterator can result in an infinite loop.";
            break;
        default:
            this->message = "Unknown error.";
            break;
        }
    }
    explicit Exception( const char* message ) {
        this->message = message;
        this->ex = std::exception();
    }
public: 
    const char* what() const noexcept override {
        return this->message.c_str();
    }
};

#endif // UTILITY_H