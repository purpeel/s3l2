#ifndef VFS_H
#define VFS_H

#include <filesystem>
#include <format>
#include <fstream>
#include "VFSNode.hpp"

namespace fs = std::filesystem;

template <template<COrdered,class> class TContainer >
class VFS
{
private: 
    using Node = VFSNode<TContainer>;
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
        _currentDir = findByPath( normalizedPath(path) );
    }

    void mkdir( const std::string& path ) {
        auto npath = normalizedPath(path);

        std::string name;
        for (size_t i = npath.rfind('/'); i < npath.length(); i++) {
            name += path[i];
        }

        if (name.empty()) {
            throw Exception( Exception::ErrorCode::INVALID_INPUT );
        } else {
            
            std::string location;
            for (size_t i = 0; i < npath.rfind('/'); i++) {
                location += path[i];
            }

            auto node = findByPath(location);
            if (!node->isDir()) {
                throw Exception( std::format("Error. {} is not a directory.", node->name()));
            } else {
                if (node->contents().contains( name )) {
                    throw Exception( std::format("Error. {} already exists.", npath));
                } else {
                    auto dir = makeShared<Dir<TContainer>>( ++_lastID, node->id(), name );
                    node->contents().add( Pair<std::string,NodeID>( name, _lastID ));
                    _data.add( Pair<NodeID,SharedPtr<Node>>( _lastID, dir ) );
                }
            }
        }        
    }
    
    void touch( const std::string& path ) {
        auto phys = fs::absolute(newTempPath()).lexically_normal();
        createTempFile(phys);

        attach( path, phys );
    }

    void attach( const std::string& vpath, const std::string& path ) {
        auto physPath = resolvePhys(fs::path( path ));
        auto npath    = normalizedPath(vpath);
        
        if (!fs::is_regular_file(physPath)) {
            throw Exception( std::format("Error. For attach {} must be a regular file.", path));
        } else {
            std::string name, extension;
            for (size_t i = npath.rfind('/'); i < npath.length(); i++) {
                name += npath[i];
                
            }
            for (size_t i = npath.rfind('.'); i < npath.length(); i++) {
                extension += npath[i];
            }

            if (name.empty()) {
                throw Exception( Exception::ErrorCode::INVALID_INPUT );
            } else {

                std::string location;
                for (size_t i = 0; i < npath.rfind('/'); i++) {
                    location += vpath[i];
                }

                auto node = findByPath(location);
                if (!node->isDir()) {
                    throw Exception( std::format("Error. {} is not a directory.", node->name()));
                } else {
                    if (node->hasChild(name)) {
                        throw Exception( std::format("Error. {} already exists.", npath));
                    } else {
                        auto file = makeShared<File<TContainer>>( ++_lastID, node->id(), name, extension, physPath );
                        node->contents().add( Pair<std::string,NodeID>( name, _lastID ));
                        _data.add( Pair<NodeID,SharedPtr<Node>>( _lastID, file ) );
                    }
                }
            }   
        }
    }

    void rmdir( const std::string& path ) {
        auto npath = normalizedPath(path);
        auto node = findByPath(npath);

        if (!node->isDir()) {
            throw Exception( std::format( "Error. {} is not a directory.", npath ) );
        }
        if (node->parent() == 0) {
            throw Exception( std::format( "Error. {} is a root directory and cannot be deleted.", npath ) );
        } else {
            auto parent = _data.get( node->parent() );
            _data.remove( node->id() );
            parent->contents().remove( node->name() );
        }
    }

    void remove( const std::string& path ) {
        auto npath = normalizedPath(path);
        auto node = findByPath(npath);

        if (node->isDir()) {
            throw Exception( std::format( "Error. {} is a directory. Use rmdir instead.", npath ) );
        } else {
            auto parent = _data.get( node->parent() );
            parent->contents().remove( node->name() );
            _data.remove( node->id() );
        }
    }

    // void move( const std::string& from, const std::string& to ) {
    //     auto node    = findByPath( normalizedPath(from) );
    //     auto destDir = findByPath( normalizedPath(to) );

    //     if (!destDir->isDir()) {
    //         throw Exception( std::format("Error. Unable to move to {}: {} is not a directory.", destDir ));
    //     } else {
    //         if (node->id() == destDir->id()) {
    //             throw Exception( Exception::ErrorCode::CYCLIC_MOVE );
    //         } else {
    //             auto srcDir = _data.get( node->parent() );
    //             srcDir->contents().remove( node->id() );

    //             if (srcDir->contents().contains( node->name() ) ) {
    //                 throw Exception( std::format("Error. {} already exists.", to));
    //             }
    //             srcDir->contents().add( Pair<std::string,NodeID>( node->name(), node->id() ));
    //         }
    //     }
    // }

    // void rename( const std::string& path, const std::string& name ) {
    //     move( path, name );
    // }
private:
    SharedPtr<Node> findByPath( const std::string& path ) {
        if (path.starts_with('/')) {
            return resolve( _rootDir, path );
        } else { 
            return resolve( _currentDir, path );
        }
    }

    SharedPtr<Node> resolve( SharedPtr<Node> node, const std::string& path ) {
        auto npath = normalizedPath(path);
        if (npath.empty()) { return node; }
        std::string token;
        SharedPtr<Node> res = node;
        for (size_t i = 0; i < npath.length(); i++) {
            if (npath[i] != '/' && npath[i] != '\0') {
                token += npath[i];
            } else {
                if (token.empty()) { continue; }
                if (token == ".." ) {
                    if (res->parent() != 0) {
                        res = _data.get(res->parent());
                    }
                } else if (res->isDir()) {
                    if (res->hasChild(token)) {
                        res = _data.get( res->child(token) );
                    } else {
                        throw Exception( std::format("Error. Resolve failed: no such file or directory: {}.", path) );
                    }
                } else {
                    throw Exception( std::format("Error. Resolve failed: {} is not a directory.", res->name()));
                }
                token = "";
            }
        }
        return res;
    }

    std::string normalizedPath( const std::string& path ) {
        if (path.empty()) { return ""; }

        ArraySequence<std::string> tokens;
        std::string token;
        bool isAbs = path[0] == '/';

        for ( size_t i = 0; i <= path.length(); i++ ) {
            if (path[i] != '/' && path[i] != '\0') {
                token += path[i];
            } else {
                if (token == "."  || token == "" 
                || (token == ".." && tokens.isEmpty() && isAbs)) {
                    token = "";
                    continue;
                } else if (token == ".." && !tokens.isEmpty() && tokens[tokens.getSize() - 1] != token) {
                    tokens.removeAt( tokens.getSize() - 1 );
                } else {
                    tokens.append( token );
                }
                token = "";
            }
        }
        std::string res = (isAbs ? "/" : "");
        for ( size_t i = 0; i < tokens.getSize(); i++ ) {
            res += (tokens[i] + "/");
        }
        if (!res.empty()) { res.pop_back(); }
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
private:
    SharedPtr<Node> _currentDir;
    SharedPtr<Node> _rootDir;
    IDictionary<NodeID,SharedPtr<Node>,TContainer<NodeID,SharedPtr<Node>>> _data;
    NodeID _lastID;

    fs::path _tempServiceDir;
    size_t _tempCount;
};

#endif // VFS_H