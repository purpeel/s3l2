#ifndef VFS_H
#define VFS_H

#if defined(__unix__) || defined(__APPLE__) 
  #include <unistd.h>
  #include <sys/wait.h>
#elif defined(_WIN32)
  #include <windows.h>
  #include <shellapi.h>
#endif
#include <filesystem>
#include <format>
#include <fstream>
#include "VFSNode.hpp"
#include "VFSPath.hpp"

namespace fs = std::filesystem;

template <template<COrdered,class> class TContainer>
class VFS
{
private: 
    using Node = VFSNode<TContainer>;
    using Path = VFSPath;
public:
    VFS() : _lastID(0), _tempCount(0) {
        _tempServiceDir = fs::current_path()/".temp";
        fs::create_directory(_tempServiceDir);
        auto root = makeShared<Dir<TContainer>>( 1, 0, "/" );
        
        _currentDir = root;
        _rootDir    = root;
        
        _data.add(++_lastID, root);
    }

    VFS( const VFS& other ) = delete;
    VFS& operator=( const VFS& other ) = delete;
    
    VFS( VFS&& other ) = delete;
    VFS& operator=( VFS&& other ) = delete;

    ~VFS() = default;
public:
    void cd( const std::string& path ) {
        auto node = findByPath(path);
        if (!node->isDir()) {
            throw Exception( std::format( "Error. cd: {} is not a directory.", Path(path).string() ));
        } else {
            _currentDir = node;
        }
    }

    void mkdir( const std::string& path ) {
        auto vpath = Path(path);
        auto name = vpath.name();

        if (name.empty()) {
            throw Exception( Exception::ErrorCode::INVALID_INPUT );
        } else {
            auto location = vpath.location();

            auto node = findByPath(location);
            if (!node->isDir()) {
                throw Exception( std::format("Error. {} is not a directory.", node->name()));
            } else {
                if (node->contents().contains( name )) {
                    throw Exception( std::format("Error. {} already exists.", vpath.string()));
                } else {
                    auto dir = makeShared<Dir<TContainer>>( ++_lastID, node->id(), name );
                    node->contents().add( Pair<std::string,NodeID>( name, _lastID ));
                    _data.add( Pair<NodeID,SharedPtr<Node>>( _lastID, dir ) );
                }
            }
        }        
    }
    
    void touch( const std::string& path ) {
        auto npath = Path(path);
        if (exists(npath)) {
            throw Exception( std::format( "Error. {} already exists.", npath.string()));
        } else {
            auto phys = fs::absolute(newTempPath()).lexically_normal();
            createTempFile(phys);
            attach( path, phys );
        }
    }

    void attach( const std::string& virtPath, const std::string& physPath ) {
        auto pPath = resolvePhys(fs::path( physPath ));
        auto vpath = Path( virtPath );
        
        if (!fs::is_regular_file(physPath)) {
            throw Exception( std::format("Error. For attach {} must be a regular file.", vpath.string()));
        } else {
            auto name = vpath.name();
            auto extension = vpath.extension();

            if (name.empty() || extension.empty()) {
                throw Exception( Exception::ErrorCode::INVALID_INPUT );
            } else {
                auto location = vpath.location();
                auto node = findByPath(location);

                if (!node->isDir()) {
                    throw Exception( std::format("Error. {} is not a directory.", node->name()));
                } else {
                    if (node->hasChild(name)) {
                        throw Exception( std::format("Error. {} already exists.", vpath.string()));
                    } else {
                        auto file = makeShared<File<TContainer>>( ++_lastID, node->id(), name + extension, physPath );
                        node->contents().add( Pair<std::string,NodeID>( name + extension, _lastID ));
                        _data.add( Pair<NodeID,SharedPtr<Node>>( _lastID, file ) );
                    }
                }
            }   
        }
    }

    void rmdir( const std::string& path ) {
        auto node = findByPath(path);
        auto vpath = Path(path);

        if (!node->isDir()) {
            throw Exception( std::format( "Error. {} is not a directory.", vpath.string() ) );
        }
        if (node->parent() == 0) {
            throw Exception( std::format( "Error. {} is a root directory and cannot be deleted.", vpath.string() ) );
        } else {
            auto parent = _data.get( node->parent() );
            auto& contents = node->contents();
            for (auto it = contents.begin(); it != contents.end(); it++) {
                _data.remove( *it );
            }
            parent->contents().remove( node->name() );
            _data.remove( node->id() );
        }
    }

    void remove( const std::string& path ) {
        auto vpath = Path(path);
        auto node = findByPath(vpath);

        if (node->isDir()) {
            throw Exception( std::format( "Error. {} is a directory. Use rmdir instead.", vpath.string() ) );
        } else {
            auto parent = _data.get( node->parent() );
            parent->contents().remove( node->name() );
            _data.remove( node->id() );
        }
    }

    void move( const std::string& from, const std::string& to ) {
        auto destPath = Path(to);

        if (exists(destPath)) {
            auto dest = findByPath(to);

            if (!dest->isDir()) {
                throw Exception( std::format("Error. Unable to move to {}: it is not a directory.", destPath.string()));
            } else {
                auto node = findByPath(from);

                if (node->id() == dest->id()) {
                    throw Exception( Exception::ErrorCode::CYCLIC_MOVE );
                } else {
                    if (node->parent() == 0) {
                        throw Exception( std::format("Error. {} is a root directory.", Path(from).string()));
                    } else {
                        if (dest->contents().contains( node->name() ) ) {
                            throw Exception( std::format("Error. {} already exists.", destPath.string()));
                        } else {
                            auto srcDir = _data.get( node->parent() );
                            srcDir->contents().remove( node->name() );
                            dest->contents().add( Pair<std::string,NodeID>( node->name(), node->id() ));
                        }
                    }
                }
            }
        } else {
            auto srcPath = Path(from);
            auto node = findByPath(srcPath);
            if (node->parent() == 0) {
                throw Exception( std::format("Error. {} is a root directory.", Path(from).string() ));
            } else {
                auto destDir = findByPath( destPath.location() );
                if (destDir->contents().contains( destPath.name() )) {
                    throw Exception( std::format( "Error. {} already exists.", destPath.string()));
                } else {
                    auto srcDir = findByPath(srcPath);
                    if (node->isDir()) {
                        auto contents = std::move(node->contents());
                        auto node1 = makeShared<Dir<TContainer>>( node->id(), node->parent(), destPath.name(), contents );
                        srcDir->contents().remove( node->name() );
                        destDir->contents().add( Pair<std::string,NodeID>( destPath.name(), node->id()));
                        _data.get(node->id()) = node1;
                    } else {
                        auto physPath = node->path();
                        auto node1 = makeShared<File<TContainer>>( node->id(), node->parent(), destPath.name(), physPath );
                        srcDir->contents().remove( node->name() );
                        destDir->contents().add( Pair<std::string,NodeID>( destPath.name(), node->id()));
                        _data.get(node->id()) = node1;
                    }
                }

            }
        }
    }

    void open( const std::string& path ) {
        auto npath = Path(path);
        auto node = findByPath( npath );

        if (node->isDir()) {
            cd( path );
        } else {
            if (!tryOpen( *node )) {
                throw Exception( Exception::ErrorCode::UNKNOWN_ERROR );
            }
        }
    }

    void ls() {
        
    }

    std::string getCD() {
        return _currentDir->name();
    }
private:
  #ifdef _WIN32
    bool tryOpen( const Node& node ) {
        auto inst = ShellExecuteW
            (
                nullptr,
                L"open",
                node->path().wstring().c_str(),
                nullptr,
                nullptr,
                SW_SHOWNORMAL
            );
        
        if ((INT_PTR) inst <= 32) {
            throw Exception( std::format(
                "Error. WinApi failed to open a file {} with code error = {}"
                , node->path().string(), (INT_PTR) inst) );
        } else return true;
    }
  #else 
    bool tryOpen( const Node& node ) {
        auto p = node.path();
        const char* path = p.c_str();


        pid_t pid = fork();
        if (pid == -1 ) {
            throw Exception( Exception::ErrorCode::FORK_FAILURE );
        }

        if (pid == 0) {
          #ifdef __APPLE__
            execl( "/usr/bin/open"
                 , "open"
                 ,"-W"
                 , path
                 , (char*) nullptr );
            _exit(127);
          #elif defined(__linux__)
            execlp( "xdg-open"
                  , "xdg-open"
                  , path
                  , (char*) nullptr );
                  
            _exit(127);
          #else
            _exit(127);
          #endif
        }

        int status = 0;
        if (waitpid(pid, &status, 0) == -1) {
            throw Exception(Exception::ErrorCode::WAITPID_FAILURE );
        }

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code == 127) {
                throw Exception( Exception::ErrorCode::EXEC_FAILURE );
            } else if (code != 0) {
                throw Exception( std::format( "Error. open tool failed with code {}", code) );
            } else return true;
        } else if (WIFSIGNALED(status)) {
            throw Exception( std::format( "Error. Child kiiled by signal with code {}", WTERMSIG(status)));
        } else {
            return false;
        }
    }
  #endif

    bool exists( const std::string& path ) const {
        auto npath = Path(path);
        auto location = npath.location();
        auto name = npath.name();

        auto node = findByPath(location);
        if (node->isDir() && node->hasChild( name )) {
            return true;
        } else return false;
    }

    bool exists( const Path& path ) const {
        auto location = path.location();
        auto name = path.name();

        auto node = findByPath(location);
        if (node->isDir() && node->hasChild( name )) {
            return true;
        } else return false;
    }

    SharedPtr<Node> findByPath( const std::string& path ) const {
        return findByPath( Path(path) );
    }

    SharedPtr<Node> findByPath( const Path& path ) const {
        if (path.isAbsolute()) {
            return resolve( _rootDir, path );
        } else { 
            return resolve( _currentDir, path );
        }
    }

    SharedPtr<Node> resolve( SharedPtr<Node> node, const Path& path ) const {
        if (path.isEmpty()) { return node; }

        SharedPtr<Node> res = node;
        
        for (size_t i = 0; i < path.getSize(); i++) {
            auto token = path[i];
            if (token == "..") {
                if (res->parent() != 0) {
                    res = _data.get( res->parent() );
                }
            } else if (res->isDir()) {
                if (token == "/") continue;
                if (res->hasChild(token)) {
                    res = _data.get( res->child(token));
                } else {
                    throw Exception( std::format("Error. Resolve failed: no such file or directory: {}", path.string()));
                }
            } else {
                throw Exception( std::format("Error. Resolve failed: {} is not a directory.", res->name()));
            }
        }
        return res;
    }
private:
    fs::path resolvePhys( const fs::path& phys ) {
        if (phys.is_absolute()) {
            if (fs::exists(phys)) {
                return phys;
            } else {
                throw Exception( std::format("Error. {} doesn't exist.", phys.string() ));
            }
        } else {
            throw Exception( Exception::ErrorCode::RELATIVE_PHYSICAL_PATH );
        }
    }
    
    fs::path newTempPath() {
        return _tempServiceDir/(std::format("temp({})", _tempCount++));
    }

    void createTempFile( const fs::path& path ) {
        std::ofstream ofs(path);
        if (!ofs) {
            throw Exception( Exception::ErrorCode::ERROR_CREATING_FILE );
        }
    }

    // std::string serialize() const {
    //     std::string result = std::format("lastID:{};currentDir:{};\n", _lastID, _currentDir->id());
    //     for (auto it = _data.begin(); it != _data.end(); ++it) {
    //         auto node = *it;
    //         if (node->isDir()) {
    //             result += std::format("D:{}:{}:{}\n", node->id(), node->parent(), node->name());
    //         } else {
    //             result += std::format("F:{}:{}:{}:{}\n", node->id(), node->parent(), node->name(), node->path());
    //         }
    //     }
    //     return result;
    // }
private:
    SharedPtr<Node> _currentDir;
    SharedPtr<Node> _rootDir;
    IDictionary<NodeID,SharedPtr<Node>,TContainer<NodeID,SharedPtr<Node>>> _data;
    NodeID _lastID;

    fs::path _tempServiceDir;
    size_t _tempCount;
};

#endif // VFS_H


// ñ++ fqa