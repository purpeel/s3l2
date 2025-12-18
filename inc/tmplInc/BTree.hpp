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
        bool isFull() const noexcept { return _children.getSize() == Fanout; } //equals _keys.getSize() == 2*Degree - 1
        bool hasNoKeys()  const noexcept { return _keys.getSize() == 0; }
        bool hasMinKeys() const noexcept { return _keys.getSize() == Degree - 1; }
        bool canAddKey()  const noexcept { return _keys.getSize() < 2 * Degree - 1; }

        attrT keysCount()  const noexcept { return _keys.getSize(); }
        attrT childCount() const noexcept { return _children.getSize(); }
        K maxKey() const noexcept { return _isSet ? _keys[keysCount() - 1] : _keys[keysCount() - 1].first(); }
        K minKey() const noexcept { return _isSet ? _keys[0]               : _keys[0].first(); }
        K midKey() const noexcept { return _isSet ? _keys[keysCount() / 2] : _keys[keysCount() / 2].first(); }
        K ithKey( const attrT& index ) const noexcept { return _isSet ? _keys[index] : _keys[index].first(); }

        SharedPtr<Node>& kthChild( const K& key ) { return _children[BSearchInChildren(key)]; }
        SharedPtr<Node>& ithChild( const attrT& index ) { return _children[index]; }
    public:
        attrT BSearchInKeys( const K& key ) { // returns index in [0, 2 * Degree - 2] and -1 in case key not found
            if (isEmpty()) { return -1; }
            attrT l = -1;
            attrT r = keysCount();
            attrT m = (r + l) / 2;

            K M = ithKey(m);

            while (l < r - 1) {
                if (M == key) { return m; }
                if (M < key) {
                    l = m;
                } else {
                    r = m;
                }
                m = (r + l) / 2;
                M = ithKey(m);
            }
            return -1;
        }
        attrT BSearchInChildren( const K& key ) { // returns index in [0, Fanout - 1]
            if (isEmpty()) { return 0; }
            attrT l = 0; 
            attrT r = childCount();
            attrT m = (r + l) / 2;
            
            K L = minKey();
            K R = maxKey();
            K M = ithKey(m);

            if (key < L) { return 0; } 
            if (key > R) { return r - 1; } 

            while (l < r - 1) {
                if (M < key) {
                    l = m;
                } else {
                    r = m;
                }
                m = (r + l) / 2;
                M = ithKey(m);
            }
            return r;   
        }
        bool hasKey( const K& key ) { return (BSearchInKeys(key) != -1); }
        bool hasInChildren( const K& key ) {
            while (true) {
                auto childToCheck = kthChild(key);
                if (childToCheck->isLeaf()) { 
                    return (childToCheck->hasKey(key));
                }
            }
        }
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
        bool isEnd() { return _atEnd(); }
        bool isBegin() { return _atBegin; }
    public:
        BTreeIterator& goDownLeft() noexcept { // concerned if all these things should return just value and not a reference
            while (!_observed->isLeaf()) {
                _observed = _observed->_children[0];
            }
            _indexInNode = 0;
            return *this;
        }
        BTreeIterator& goDownRight() noexcept {
            while (!_observed->isLeaf()) {
                _observed = _observed->_children[_observed->childCount() - 1];
            }
            _indexInNode = _observed->keysCount() - 1;
            return *this;
        }
        BTreeIterator& goUp() noexcept {
            while (_observed->_parent) {
                _observed = _observed->_parent;
            }
            return *this;
        }
        BTreeIterator& stepForward() noexcept {
            if (_atEnd) { return *this; }
            if (_atBegin) { _atBegin = false; }
            if (_observed->isLeaf()) {
                if (_indexInNode < _observed->keysCount()) {
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
                _observed = _observed->_children[_indexInNode];
                return goDownLeft();
            }
            return *this;
        }
        BTreeIterator& stepBack() noexcept {
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
                _observed = _observed->_children[_indexInNode];
                return goDownRight();
            }
            return *this;
        }
        SharedPtr<Node>& observed() { return _observed; }
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
            if (node->hasInChildren(key)) {
                return get( node->kthChild(key), key );
            }
        } else {
            return iterT( node, searchInKeysRes );
        }
    }
    constIterT get( SharedPtr<Node> node = _root, const K& key ) const {
        auto searchInKeysRes = node->BSearchInKeys( key );
        if ( searchInKeysRes == -1 ) {
            if (node->hasInChildren(key)) {
                return get( node->kthChild(key), key );
            }
        } else {
            return constIterT( node, searchInKeysRes );
        }
    }
    template <bool isSet = _isSet> requires(isSet)  
    BTree& insert( const V& value ) {
        return insertInSubtree( _root, Pair<V,V>( value, value ) );
    }
    BTree& insert( const Pair<K,V>& pair ) {
        return insertInSubtree( _root, pair );
    }
    BTree& remove( const K& key ) {
        
    }

    BTree& removeFromSubtree( SharedPtr<Node>& node, const K& key ) {
        if (node->isLeaf()) {
            if (node->hasKey(key)) {
                if (!node->_parent) {
                    node->_keys.removeAt(node->BSearchInKeys(key));
                } else {
                    if (node->hasMinKeys()) {
                        attrT indexInParent = node->_parent->BSearchInChildren(key);
                        if (indexInParent > 0 && indexInParent < node->_parent->keysCount()) {
                            auto& left   = node->_parent->ithChild(indexInParent - 1);
                            auto& right  = node->_parent->ithChild(indexInParent + 1);
                            node->_keys.removeAt(node->BSearchInKeys(key));
                            if (left->hasMinKeys() && right->hasMinKeys()) { return merge( left, node ); } 
                            else if (!left->hasMinKeys())  { return  rotateLeft( node ); } 
                            else                           { return rotateRight( node ); }
                        } else if (indexInParent == 0) {
                            auto& right  = node->_parent->ithChild(indexInParent + 1);
                            node->_keys.removeAt(node->BSearchInKeys(key));
                            if (right->hasMinKeys()) { return merge( node, right ); } 
                            else                     { return rotateRight( node ); }
                        } else {
                            auto& left  = node->_parent->ithChild(indexInParent - 1);
                            node->_keys.removeAt(node->BSearchInKeys(key));
                            if (left->hasMinKeys()) { return merge( left, node ); } 
                            else                    { return rotateLeft( node ); }
                        }
                    } else {
                        node->_keys.removeAt( node->BSearchInKeys(key) );
                    }
                }
            }
        } else {
            if (!node->hasKey(key)) {
                if (!node->_parent || !node->hasMinKeys()) {
                    return removeFromSubtree( node->ithChild(BSearchInChildren(key)), key );
                } else {
                    attrT indexInParent = node->_parent->BSearchInChildren(key);
                    if (indexInParent > 0 && indexInParent < node->_parent->keysCount()) {
                        auto& left   = node->_parent->ithChild(indexInParent - 1);
                        auto& right  = node->_parent->ithChild(indexInParent + 1);
                        if (left->hasMinKeys() && right->hasMinKey()) {
                            return merge(left, node)
                                  .removeFromSubtree(
                                    node->_parent->ithChild(indexInParent - 1), key
                                                     );
                        } else if (!right->hasMinKeys()) { 
                            return rotateRight(node)
                                  .removeFromSubtree(
                                    node->_parent->ithChild(indexInParent), key
                                                     );
                        } else {
                            return rotateLeft(node)
                                  .removeFromSubtree(
                                    node->_parent->ithChild(indexInParent), key 
                                                     );
                        }
                    } else if (indexInParent == 0) {
                        auto& right  = node->_parent->ithChild(indexInParent + 1);
                        if (right->hasMinKeys()) {
                            return merge(node, right)
                                  .removeFromSubtree(
                                    node->_parent->ithChild(indexInParent), key
                                                     );
                        } else {
                            return rotateRight(node)
                                  .removeFromSubtree(
                                    node->_parent->ithChild(indexInParent), key
                                                     );
                        }
                    } else {
                        auto& left  = node->_parent->ithChild(indexInParent - 1);
                        if (left->hasMinKeys()) {
                            return merge(left, node)
                                  .removeFromSubtree(
                                    node->_parent->ithChild(indexInParent - 1), key
                                                     );
                        } else {
                            return rotateLeft(node)
                                  .removeFromSubtree(
                                    node->_parent->ithChild(indexInParent), key
                                                     );
                        }
                    }
                }
            } else {
                if (node->hasMinKeys()) {

                } else {
                    auto it1 = iterT(node, BSearchInKeys(key)), it2 = iterT(node, BSearchInKeys(key));
                    auto predecessor = (--it1).observed();     
                    auto successor = (++it2).observed();
                    if (predecessor->hasMinKeys() && successor->hasMinKeys()) {
                        
                    } else if (!predecessor->hasMinKeys()) {
                        K maxKey = predecessor->maxKey();
                        // need to be able to do a half-rotate but not with sibling and with successor or predecessor for rearranging keys correctly
                    }
                }

            }
        }
    }

    BTree& rotateLeft( SharedPtr<Node>& node ) {
        auto& parent = node->_parent;
        attrT indexInParent = parent->BSearchInChildren(node->minKey()) - 1;
        if (indexInParent >= 0) {
            auto& leftSibling = parent->ithChild(indexInParent);
            node->_keys.prepend(parent->ithKey(indexInParent));
            parent->_keys.removeAt(indexInParent);
            parent->_keys.insertAt(leftSibling->maxKey(), indexInParent)
            if (!node->isLeaf()) {
                node->_children.prepend(leftSibling->_children[leftSibling->childCount() - 1]);
                leftSibling->_children.removeAt(leftSibling->childCount() - 1);
            }
            leftSibling->_keys.removeAt(leftSibling->keysCount() - 1);
        }
        return *this;
    }   

    
    BTree& rotateRight( SharedPtr<Node>& node ) {
        auto& parent = node->_parent;
        attrT indexInParent = parent->BSearchInChildren(node->maxKey());
        if (indexInParent < parent->childCount() - 1) {
            auto rightSibling = parent->ithChild(indexInParent + 1);
            node->_keys.append(parent->ithKey(indexInParent));
            parent->_keys.removeAt(indexInParent);
            parent->_keys.insertAt(rightSibling->minKey(), indexInParent)
            if (!node->isLeaf()) {
                node->_children.append(rightSibling->_children[0]);
                rightSibling->_children.removeAt(0);
            }
            rightSibling->_keys.removeAt(0);
        }
        return *this;
    }    
    
    BTree& merge( SharedPtr<Node>& node1, SharedPtr<Node>& node2 ) {
        auto& parent = node1->_parent;
        attrT sepIndex = parent->BSearchInChildren( node1->maxKey() );
        K separator = parent->ithKey( sepIndex );
        parent->_keys.removeAt(sepIndex);
        node1->_keys.append(separator);
        node1->_keys.concat( node2->_keys );
        node1->_children.concat( node2->_children );
        parent->_children.removeAt(sepIndex + 1);
        return *this;
    }
    
    BTree& split( SharedPtr<Node>& node ) {
        SharedPtr<Node> left, right;
        if (!node->_parent) {
            node->_parent = makeShared<Node>();
            _root = node->_parent;
        }

        auto& parent = node->_parent;
        left->_parent = right->_parent = parent;
        left->_keys = node->_keys.subArray(0, node->keysCount()/2);
        right->_keys = node->_keys.subArray(node->keysCount()/2 + 1, node->keysCount());

        auto indexInParent = parent->BSearchInChildren(node->midKey());
        parent->_keys.insertAt(node->midKey(), indexInParent);
        parent->_children[indexInParent] = left;
        parent->_children.insertAt(right, indexInParent + 1); 
        return *this     
    }

    BTree& insertInSubtree( SharedPtr<Node>& root, const Pair<K,V>& pair ) {
        if (root->isLeaf()) {
            if (root->hasKey(pair.first())) {
                throw Exception( Exception::ErrorCode::KEY_COLLISION );
            } else {
                root->_keys.insertAt(pair, root->BSearchInChildren(pair.first()));
            }
            return *this;
        }
        if (root->isFull()) {
            return split(root).insertInSubtree(
                root->ithChild( root->BSearchInChildren(pair.first()) ), pair
                                               );
        } else {
            return insertInSubtree(
                root->ithChild( root->BSearchInChildren(pair.first()) ), pair
                                   );
        }
    }

    bool contains( const K& key ) const;
    bool isEmpty() const;
};

#endif // BTREE_H