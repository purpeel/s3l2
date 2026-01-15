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
    V& get( const K& key ) requires CChageableByKey<TContainer,K,V>
    {
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
    struct constIterTraits {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type = V;
        using pointer    = const V*;
        using reference  = const V&;
    };
    struct nonConstIterTraits {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type = V;
        using pointer    = V*;
        using reference  = V&;
    };
    template <typename IterTraits> 
    class IDictionaryIterator 
    {
    public:
        using iterator_category = typename IterTraits::iterator_category;
        using difference_type   = typename IterTraits::difference_type;
        using value_type = typename IterTraits::value_type;   
        using pointer    = typename IterTraits::pointer;   
        using reference  = typename IterTraits::reference; 
    public:
        IDictionaryIterator() = default;
        IDictionaryIterator( const TContainer::TIter& iter ) 
        : _iter( iter ) {}
        template <typename otherTraits>
        IDictionaryIterator& operator=( const IDictionaryIterator<otherTraits>& other ) {
            _iter = other._iter;
        }
    public:
        pointer operator->() noexcept {
            return _iter.operator->();
        }
        reference operator*() noexcept {
            return _iter.operator*();
        }
    public:
        IDictionaryIterator& operator++() noexcept {
            ++_iter;
            return *this;
        }
        IDictionaryIterator operator++(int) noexcept {
            auto temp = *this;
            ++_iter;
            return *this;
        }
        IDictionaryIterator& operator--() noexcept {
            --_iter;
            return *this;
        }
        IDictionaryIterator operator--(int) noexcept {
            auto temp = *this;
            --_iter;
            return *this;
        }
        friend bool operator==( const IDictionaryIterator& lhs, const IDictionaryIterator& rhs ) {
            return lhs._iter == rhs._iter;
        }
        friend bool operator!=( const IDictionaryIterator& lhs, const IDictionaryIterator& rhs ) {
            return lhs._iter != rhs._iter;
        }
    private:
        TContainer::TIter _iter;
    };
public:
    using TIter = IDictionaryIterator<std::conditional_t<
                            std::is_same_v<K,V>,constIterTraits,nonConstIterTraits
                                                         >>;
    using constTIter = IDictionaryIterator<constIterTraits>;

    TIter begin() { return TIter(_container.begin()); }
    TIter end()   { return TIter(_container.end()); }
    constTIter begin() const { return constTIter(_container.begin()); }
    constTIter end() const   { return constTIter(_container.end()); }
private:
    TContainer _container;
    ssize_t _capacity;
};

#endif // IDICTIONARY_H
