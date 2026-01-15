#ifndef VFSPATH_H
#define VFSPATH_H

#include "ArraySequence.hpp"

class VFSPath
{
public:
    VFSPath() {
        _tokens.append("/");
    }
    VFSPath( const std::string& path ) {
        normalize(path);
    }

    VFSPath( const VFSPath& other ) = default;
    VFSPath& operator=( const VFSPath& other ) = default;

    VFSPath( VFSPath&& other ) = default;
    VFSPath& operator=( VFSPath&& other ) = default;

    ~VFSPath() = default;
public:
    std::string string() const noexcept {
        std::string res;
        for (size_t i = 0; i < getSize(); i++) {
            res += _tokens[i];
            if (i != getSize() - 1) res += "/";
        }
        return res;
    }
    std::string operator[]( const size_t index ) const {
        return _tokens[index];
    }
    std::string name() const noexcept {
        return extractName(_tokens[_tokens.getSize() - 1]);
    }
    std::string extension() const noexcept {
        return extractExtension(_tokens[_tokens.getSize() - 1]);
    }
    VFSPath location() const noexcept {
        VFSPath loc;
        if (_tokens.getSize() != 0) {
            loc._tokens = _tokens.subArray(0, _tokens.getSize() - 1);
        }
        
        return loc;
    }
    size_t getSize() const noexcept {
        return _tokens.getSize();
    }
    
    bool isAbsolute() const noexcept {
        if (_tokens.isEmpty()) return false;
        else return _tokens[0] == "/";
    }
    bool isToFile() const noexcept {
        return !extractExtension(_tokens[_tokens.getSize() - 1]).empty();
    }
    bool isToFolder() const noexcept {
        return extractExtension(_tokens[_tokens.getSize() - 1]).empty();
    }
    bool isEmpty() const noexcept {
        return _tokens.isEmpty();
    }
public:
    VFSPath& operator+=( const std::string& other ) {
        auto otherPath = VFSPath(other);

        return (*this += otherPath);
    }
    VFSPath& operator+=( const VFSPath& other ) {
        if (other.isAbsolute()) {
            throw Exception( Exception::ErrorCode::CONCAT_WITH_ABS_PATH );
        } else {
            _tokens.concat(other._tokens);
        }
        return *this;
    }
    VFSPath& operator/( const std::string& other ) {
        return (*this += other);
    }
    VFSPath& operator/( const VFSPath& other ) {
        return (*this += other);
    }

    friend bool operator==( const VFSPath& lhs, const VFSPath& rhs ) {
        if (lhs._tokens.getSize() == rhs._tokens.getSize()) {
            for (size_t i = 0; i < lhs._tokens.getSize(); i++) {
                if (lhs._tokens[i] != rhs._tokens[i]) return false;
            }
            return true;
        } else return false;
    }
    friend bool operator!=( const VFSPath& lhs, const VFSPath& rhs ) {
        return !(lhs == rhs);
    }
private:
    void normalize( const std::string& path ) {
        if (path.empty()) { return; }

        ArraySequence<std::string> stack;
        std::string token;
        bool isAbs = path[0] == '/';

        for ( size_t i = 0; i <= path.length(); i++ ) {
            if (path[i] != '/' && path[i] != '\0') {
                token += path[i];
            } else {
                if (token == "."  || token == "" 
                || (token == ".." && stack.isEmpty() && isAbs)) {
                    token = "";
                    continue;
                } else if (token == ".." && !stack.isEmpty() && stack[stack.getSize() - 1] != token) {
                    stack.removeAt( stack.getSize() - 1 );
                } else {
                    stack.append( token );
                }
                token = "";
            }
        }
        if (isAbs) _tokens.append("/");
        for ( size_t i = 0; i < stack.getSize(); i++ ) {
            _tokens.append(stack[i]);
        }
    }
    static std::string extractExtension( const std::string& title ) {
        std::string ext;
        for (size_t i = title.rfind('.'); i < title.length(); i++) {
            ext += title[i];
        }
        return ext;
    }
    static std::string extractName( const std::string& title ) {
        std::string name;
        size_t end = title.contains('.') ?  title.rfind('.') : title.size();
        for (size_t i = title.rfind('/') + 1; i < end; i++) {
            name += title[i];
        }
        return name;
    }
private:
    ArraySequence<std::string> _tokens;
};


#endif // VFSPATH_H
