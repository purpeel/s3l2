#ifndef VFSNODE_H
#define VFSNODE_H

#include "IDictionary.hpp"
#include "util.hpp"
#include <filesystem>

using NodeID = std::size_t;

template <template<COrdered,class> class TContainer>
struct VFSNode
{
    VFSNode( const NodeID id, const NodeID parent, const std::string& name )
    : _id(id), _parentID(parent), _name(name) {}

    virtual bool isDir() const { return false; }
    virtual NodeID parent() const { return _parentID; }
    virtual NodeID id() const { return _id; }
    virtual std::string name() const { return _name; }

    virtual NodeID child( const std::string& name ) const { 
        throw Exception( std::format( "Error. {} is not a directory and can't contain {}.", _name, name ) ); 
    }
    virtual bool hasChild( const std::string& name ) const { 
        throw Exception( std::format( "Error. {} is not a directory and can't contain {}.", _name, name ) ); 
    }    
    virtual IDictionary<std::string,NodeID,TContainer<std::string,NodeID>>& contents() {
        throw Exception( std::format( "Error. {} is not a directory." , _name) );
    }
    virtual const std::string& ext() const {
        throw Exception( std::format( "Error. {} is not a regular file.", _name ) ); 
    }
    virtual const std::filesystem::path& path() const {
        throw Exception( std::format( "Error. {} is not a regular file.", _name ) ); 
    } 
    virtual ~VFSNode() = default;

    NodeID _id;
    NodeID _parentID;
    std::string _name;
};

template <template<COrdered,class> class TContainer>
struct Dir : VFSNode<TContainer>
{
    using Dict = IDictionary<std::string,NodeID,TContainer<std::string,NodeID>>;

    Dir( const NodeID id, const NodeID parent, const std::string& name )
    : VFSNode<TContainer>( id, parent, name ) {}

    Dir( const NodeID id, const NodeID parent, const std::string& name, Dict& contents )
    : VFSNode<TContainer>( id, parent, name ), _contents( std::move(contents) ) {}

    bool isDir() const override { return true; }
    NodeID child( const std::string& name ) const override { return _contents.get(name); }
    bool hasChild( const std::string& name ) const override { return _contents.contains(name); } 
    virtual Dict& contents() { return _contents; }
    
    ~Dir() = default;

    Dict _contents; 
};


template <template<COrdered,class> class TContainer>
struct File : VFSNode<TContainer>
{ 
    File( const NodeID id, const NodeID parent, const std::string& name
        , const std::filesystem::path& path )
    : VFSNode<TContainer>( id, parent, name ) 
    , _diskPath( path ) {}
    
    const std::filesystem::path& path() const override { return _diskPath; } 

    ~File() = default;

    std::filesystem::path _diskPath;
};

#endif // VFSNODE_H 