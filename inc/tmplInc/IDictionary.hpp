#ifndef IDICTIONARY_H
#define IDICTIONARY_H

#include "CRequirements.hpp"
#include "BTree.hpp"
#include "BPlusTree.hpp"

template <typename K, typename V, typename TContainer = BTree<K,V>>     
requires CAssociative<TContainer,K,V>
class IDictionary 
{
public:
    IDictionary() : _container(), _capacity() {}
    IDictionary( const ssize_t capacity ) 
    : _container(), _capacity( capacity ) {}

    IDictionary( const IDictionary& other ) = delete;
    IDictionary& operator=( const IDictionary& other ) = delete;

    IDictionary( IDictionary&& other ) = default;
    IDictionary& operator=( IDictionary&& other ) = default;

    ~IDictionary() = default;
public:
    template <bool isSet> requires CChageableByKey<TContainer,K,V>
    V& get( const K& key ) {
        return _container.get(key);
    }
    const V& get( const K& key ) const {
        return _container.get(key);
    }
    void add( const Pair<K,V>& pair ) {
        _container.insert(pair);
    }
    void add( const K& key, const V& value ) {
        auto pair = Pair<K,V>( key, value );
        _container.insert(pair);
    }
    void remove( const K& key ) {
        _container.remove(key);
    }
    bool contains( const K& key ) const {
        return _container.contains(key);
    }
public:
    ssize_t getSize() const noexcept {
        return _container.getSize();
    }
    ssize_t getCapacity() const noexcept {
        return _capacity;
    }
    bool isEmpty() const noexcept {
        return _container.isEmpty();
    }
private:
    TContainer _container;
    ssize_t _capacity;
};

#endif // IDICTIONARY_H
