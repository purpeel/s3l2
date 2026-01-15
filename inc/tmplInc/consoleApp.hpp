#ifndef VFS_CONSOLE_H
#define VFS_CONSOLE_H

#include "VFS.hpp"
#include <iostream>

template <template<COrdered,class> class TContainer >
class VFSConsoleApp
{
public:
    VFSConsoleApp()
    : _vfs( VFS<TContainer>() ) {}

    ~VFSConsoleApp() = default;
public:
    void execute( const std::string& input ) {
        auto inputs = split( removeExtraSpaces(input) );
        if (inputs.getSize() == 0) {
            throw Exception( Exception::ErrorCode::INVALID_INPUT );
        } else if (inputs.getSize() == 1) {
            if (inputs[0] == "help" || inputs[0] == "h" ) {
                printManual();
            } else if (inputs[0] == "exit" ) {
                throw ExitSignal();
            } else {
                _vfs.open( inputs[0] );
            }
        } else if (inputs.getSize() == 2) {
            if (inputs[0] == "cd") {
                _vfs.cd( inputs[1] ); 
            } else if (inputs[0] == "rmdir") {
                _vfs.rmdir( inputs[1] );
            } else if (inputs[0] == "remove" || inputs[0] == "rm") {
                _vfs.remove( inputs[1] );
            } else if (inputs[0] == "touch") {
                _vfs.touch( inputs[1] );
            } else if (inputs[0] == "mkdir") {
                _vfs.mkdir( inputs[1] );
            } else {
                throw Exception( Exception::ErrorCode::INVALID_INPUT );
            }
        } else {
            if (inputs[0] == "move" || inputs[0] == "mv") {
                _vfs.move( inputs[1], inputs[2] ); 
            } else if (inputs[0] == "attach") {
                _vfs.attach( inputs[1], inputs[2] );
            } else {
                throw Exception( Exception::ErrorCode::INVALID_INPUT );
            }
        }
    }

    static void showError( const Exception& ex ) {
        std::cout << ex.what() << std::endl;
    }

    static void showStart() {
        std::cout << "Virtual File System Console\n";
        std::cout << "Type 'help' for available commands\n";
    }

    static std::string awaitInput() {
        std::string input;
        std::getline(std::cin, input);
        return input;
    }

    void showCurrent() {
        std::cout << _vfs.getCD() << " \033[1;32m?\033[0m ";
    }
private:
    ArraySequence<std::string> split( const std::string& input ) {
        ArraySequence<std::string> res;
        std::string token;
        for ( size_t i = 0; i <= input.length(); i++ ) {
            if ( input[i] != ' ' && input[i] != '\0' ) {
                token += input[i]; 
            } else {
                res.append(token);
                token = "";
            }
        }
        return res;
    }

    std::string removeExtraSpaces( const std::string& str ) {
        std::string res;
        bool atBegin = true;
        for ( size_t i = 0; i < str.length(); i++ ) { 
            if        (str[i] == ' ' && (str[i+1] == ' ' || str[i+1] == '\0')) {
                continue;
            } else if (str[i] == ' ' && (str[i+1] != ' ' || str[i+1] != '\0') && !atBegin) {
                res += str[i];
            } else if (str[i] != ' ' && atBegin) {
                atBegin = false;
                res += str[i];
            } else if (!atBegin) {
                res += str[i];
            }
        }
        return res;
    }

    void printManual() {
        std::cout << "VFS Commands:\n";
        std::cout << "  cd <path>              - Change directory\n";
        std::cout << "  mkdir <path>           - Create directory\n";
        std::cout << "  touch <path>           - Create empty file\n";
        std::cout << "  attach <vpath> <ppath> - Attach physical file to virtual path\n";
        std::cout << "  rmdir <path>           - Remove directory\n";
        std::cout << "  rm/remove <path>       - Remove file\n";
        std::cout << "  mv/move <from> <to>    - Move file/directory\n";
        std::cout << "  <path>                 - Open file/directory\n";
        std::cout << "  help/h                 - Show this manual\n";
        std::cout << "  exit                   - Exit application\n";
    }
private:
    VFS<TContainer> _vfs;
};

#endif // VFS_CONSOLE_H