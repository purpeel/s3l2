#ifndef BTREE_H
#define BTREE_H

#include "SharedPtr.hpp"
#include "ArraySequence.hpp"
#include "Pair.hpp"
#include "Option.hpp"
#include "BTreeDetails.hpp"

// usually Fanout = 2 * Degree
template <COrdered K, typename V, attrT Fanout = 32, attrT Degree = 16>
class BTree 
{
private:
    static constexpr bool _isSet = std::is_same_v<K,V>;

    struct Node {
    private:
        SharedPtr<Node> _parent;

        using _keysT = std::conditional_t<_isSet, V, Pair<K,V>>;
        ArraySequence<_keysT> _keys;
        ArraySequence<SharedPtr<Node>> _children;  
    public:
        Node() : _keys( ArraySequence<_keysT>() ), _children( ArraySequence<SharedPtr<Node>>() ) {}

        Node( const Node& other ) = delete;
        Node& operator=( const Node& other ) = delete;
        Node( Node&& other ) = delete;
        Node& operator=( Node&& other ) = delete;

        ~Node() = default;
    public:
        bool isLeaf() const noexcept { return _children.isEmpty(); }
        bool isFull() const noexcept { return _children.getSize() == Fanout; }
        bool hasMinKeys() const noexcept { return _keys.getSize() == Degree - 1; }
        bool canAddKey()  const noexcept { return _keys.getSize() < 2 * Degree - 1; }

        attrT keysCount()  const noexcept { return _keys.getSize(); }
        attrT childCount() const noexcept { return _children.getSize(); }
        K maxKey() const noexcept { return _isSet ? _keys[keysCount() - 1] : _keys[keysCount() - 1].first(); }
        K minKey() const noexcept { return _isSet ? _keys[0]               : _keys[0].first(); }
        K ithKey( const attrT& index ) const noexcept { return _isSet ? _keys[index] : _keys[index].first(); }

        SharedPtr<Node> kthChild( const K& key ) { return _children[BSearchInChildren(key)]; }
    public:
        attrT BSearchInKeys( const K& key ) { // returns index in [0, 2 * Degree - 2] and -1 in case key not found
            attrT l = -1;
            attrT r = keysCount();
            attrT m = (r + l) / 2;

            K L = minKey();
            K R = maxKey();
            K M = ithKey(m);

            while (l < r - 1) {
                if (m < key) {
                    l = m;
                    L = M;
                } else {
                    r = m;
                    R = M;
                }
                if (M == key) { return m; }
                m = (r + l) / 2;
            }
            return l;
        }
        attrT BSearchInChildren( const K& key ) { // returns index in [0, Fanout - 1] and -1 in case there is no children containing that key
            attrT l = -1;
            attrT r = childCount();
            attrT m = (r + l) / 2;
            
            K L = minKey();
            K R = maxKey();
            K M = ithKey(m);

            if (key < L) { return 0; } 
            if (key > R) { return r - 1; } 

            while (l < r - 1) {
                if (m < key) {
                    l = m;
                    L = M;
                } else {
                    r = m;
                    R = M;
                }
                m = (r + l) / 2;
            }
            return l;            
        }
        bool hasInKeys( const K& key ) { return (BSearchInKeys(key) != -1); }
        bool hasInChildren( const K& key ) { return (BSearchInChildren(key) != -1); }
    };
    SharedPtr<Node> _root;
private:
    struct constIterTraits {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = attrT;
        using value_type = V;
        using pointer    = const V*;
        using reference  = const V&;
    };
    struct nonConstIterTraits {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = attrT;
        using value_type = V;
        using pointer    = V*;
        using reference  = V&;
    };

    template <typename IterTraits> 
    class BTreeIterator {
    public:
        using iterator_category = typename IterTraits::iterator_category;
        using difference_type   = typename IterTraits::difference_type;
        using value_type = typename IterTraits::value_type;   
        using pointer    = typename IterTraits::pointer;   
        using reference  = typename IterTraits::reference; 
    public:
        BTreeIterator() = default;
        BTreeIterator( SharedPtr<Node> node, const attrT index, bool atBegin = false, bool atEnd = false ) 
        : _root(node), _observed(node), _indexInNode(index), _atBegin(atBegin), _atEnd(atEnd) {}

        template <typename OtherTraits>
        BTreeIterator( const BTreeIterator<OtherTraits>& other ) = default;
        template <typename OtherTraits>
        BTreeIterator& operator=( const BTreeIterator<OtherTraits>& other ) = default;
    public:
        reference operator*() const noexcept {
            return _observed->_keys[_indexInNode];
        }
        pointer operator->() const noexcept {
            return std::addressof( _observed->_keys[_indexInNode]);
        }
    
        BTreeIterator& operator++() noexcept {
            return stepForward();
        }
        BTreeIterator operator++(int) noexcept {
            auto res = *this;
            stepForward();
            return res;
        }
        BTreeIterator& operator--() noexcept {
            return stepBack();
        }
        BTreeIterator operator--(int) noexcept {
            auto res = *this;
            stepBack();
            return res;
        }
        
        friend bool operator==( const BTreeIterator& lhs, const BTreeIterator& rhs ) noexcept {
            return lhs._observed == rhs._observed && lhs._indexInNode == rhs._indexInNode;
        }
        friend bool operator!=( const BTreeIterator& lhs, const BTreeIterator& rhs ) noexcept {
            return !( lhs == rhs );
        }
    public:
        BTreeIterator& goDownLeft() noexcept { // concerned if all that things should return just value and not a reference
            while (!_observed->isLeaf()) {
                _observed = _observed->_children[0];
            }
            _indexInNode = 0;
            return *this;
        }
        BTreeIterator& goDownRight() noexcept { //^
            while (!_observed->isLeaf()) {
                _observed = _observed->_children[_observed->childCount() - 1];
            }
            _indexInNode = _observed->keysCount() - 1;
            return *this;
        }
        BTreeIterator& goUp() noexcept { //^
            while (_observed->_parent) {
                _observed = _observed->_parent;
            }
            return *this;
        }
        BTreeIterator& stepForward() noexcept { //^
            if (_atEnd) { return *this; }
            if (_atBegin) { _atBegin = false; }
            if (_observed->isLeaf()) {
                if (_indexInNode < _observed->keysCount() - 1) {
                    _indexInNode++;
                } else {
                    K maxKey = _observed->maxKey();
                    _observed = _observed->_parent;
                    while (_observed->keysCount() == _observed->BSearchInChildren( maxKey ) && _observed->_parent) {
                        _observed = _observed->_parent;
                    }
                    if (!_observed->_parent) { 
                        _atEnd = true;
                        _observed = SharedPtr<Node>();
                    } else {
                        _indexInNode = _observed->BSearchInChildren( maxKey );
                    }
                }
            } else {
                if (_indexInNode < _observed->keysCount() - 1) {
                    _observed = _observed->_children[_indexInNode + 1];
                    _indexInNode = 0;
                } else {
                    K maxKey = _observed->maxKey();
                    _observed = _observed->_parent;
                    while (_observed->keysCount() == _observed->BSearchInChildren( maxKey ) && _observed->_parent) {
                        _observed = _observed->_parent;
                    }
                    _indexInNode = _observed->BSearchInChildren( maxKey );
                }
            }
            return *this;
        }
        BTreeIterator& stepBack() noexcept { //^
            if (_atBegin) { return *this; }
            if ( _atEnd ) { 
                _atEnd = false;
                _observed = _root;
                return goDownRight(); 
            }
            if (_observed->isLeaf()) {
                if (_indexInNode > 0) {
                    _indexInNode--;
                } else {
                    auto initialLeaf = _observed;                    
                    K minKey = _observed->minKey(); 
                    _observed = _observed->_parent;
                    while (_observed->BSearchInChildren( minKey ) == 0 && _observed->_parent) {
                        _observed = _observed->_parent;
                    }
                    if (!_observed->_parent) { 
                        _atBegin = true;
                        _observed = initialLeaf;
                    }
                    _indexInNode = _observed->BSearchInChildren( minKey );
                }
            } else {
                if (_indexInNode > 0) {
                    goDownRight();
                } else {
                    K minKey = _observed->minKey();
                    _observed = _observed->_parent;
                    while (_observed->BSearchInChildren( minKey ) == 0 && _observed->_parent) {
                        _observed = _observed->_parent;
                    }
                    _indexInNode = _observed->BSearchInChildren( minKey );
                }
            }
            return *this;
        }
    private:
        SharedPtr<Node> _root;
        SharedPtr<Node> _observed;
        attrT _indexInNode;
        bool _atEnd;
        bool _atBegin;
    };
public:
    using iterT = BTreeIterator<
        std::conditional<_isSet,constIterTraits,nonConstIterTraits>
                                >;
    using constIterT = BTreeIterator<constIterTraits>;
    iterT begin() noexcept {
        auto res = iterT( _root, 0, true, false );
        res = res.goDownLeft();
        return res;
    }
    constIterT begin() const noexcept {
        auto res = constIterT( _root, 0, true, false );
        res = res.goDownLeft();
        return res;
    }
    iterT end() noexcept {
        auto res = iterT( _root, 0, false, true );
        return res.goDownRight().stepForward();
    }
    constIterT end() const noexcept {
        auto res = constIterT( _root, 0, false, true );
        return res.goDownRight().stepForward();
    }
public:
    BTree() = default;

    BTree( const BTree<K,V>& other ) = delete;
    BTree<K,V>& operator=( const BTree<K,V>& other ) = delete;
    BTree( BTree<K,V>&& other ) = default;
    BTree<K,V>& operator=( BTree<K,V>&& other ) = default;

    ~BTree() = default;
public:
    iterT get( SharedPtr<Node> node = _root, const K& key ) {
        auto searchInKeysRes = node->BSearchInKeys( key );
        if ( searchInKeysRes == -1 ) {
            auto searchInChildrenRes = node->BSearchInChildren( key );
            if ( searchInChildrenRes != -1 ) {
                return get( node->kthChild(key), key );
            }
        } else {
            return iterT( node, searchInKeysRes );
        }
    }
    constIterT get( SharedPtr<Node> node = _root, const K& key ) const {
        auto searchInKeysRes = node->BSearchInKeys( key );
        if ( searchInKeysRes == -1 ) {
            auto searchInChildrenRes = node->BSearchInChildren( key );
            if ( searchInChildrenRes != -1 ) {
                return get( node->kthChild(key), key );
            }
        } else {
            return constIterT( node, searchInKeysRes );
        }
    }
    template <bool isSet = _isSet> requires(isSet)  
    void insert( const V& value ); 
    void insert( const Pair<K,V>& pair );

    void remove( const K& key );
    void leftRotation( SharedPtr<Node> node );
    void rightRotation( SharedPtr<Node> node );

    bool contains( const K& key ) const;
    bool isEmpty() const;
    size_t getCount() const;
};

#endif // BTREE_H