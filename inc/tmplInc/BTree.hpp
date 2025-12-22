#ifndef BTREE_H
#define BTREE_H

#include "SharedPtr.hpp"
#include "ArraySequence.hpp"
#include "Pair.hpp"
#include "Option.hpp"
#include "Ordering.hpp"
#include <limits>

// degree is a tree parameter defining the minimum and maximum amount of keys per node - [t-1; 2t-1] and children per node - [t; 2t]
// in that case fanout which is max amount of children per node equals degree * 2.
template <COrdered K, typename V, ssize_t Degree = 32>
class BTree 
{
private:
    static constexpr bool _isSet = std::is_same_v<K,V>;
    static constexpr ssize_t _fanout = Degree * 2;
    static constexpr ssize_t _degree = Degree;

    struct Node {
        SharedPtr<Node> _parent;

        using _keysT = std::conditional_t<_isSet, V, Pair<K,V>>;
        ArraySequence<_keysT> _keys;
        ArraySequence<SharedPtr<Node>> _children;  
    public:
        Node() = default;

        Node( const Node& other ) = delete;
        Node& operator=( const Node& other ) = delete;
        Node( Node&& other ) = delete;
        Node& operator=( Node&& other ) = delete;

        ~Node() = default;
    public:
        bool isLeaf() const noexcept { return _children.isEmpty(); }
        bool isFull() const noexcept { return _children.getSize() == _fanout; };
        bool hasNoKeys()  const noexcept { return _keys.getSize() == 0; }
        bool hasMinKeys() const noexcept { return _keys.getSize() == _degree - 1; }
        bool canAddKey()  const noexcept { return _keys.getSize() < 2 * _degree - 1; }

        ssize_t keyCount()  const noexcept { return _keys.getSize(); }
        ssize_t childCount() const noexcept { return _children.getSize(); }
        const K& maxKey() const noexcept { 
            if constexpr (_isSet) { return _keys[keyCount() - 1]; } 
            else { return _keys[keyCount() - 1].first(); }
        }
        const K& minKey() const noexcept { 
            if constexpr (_isSet) { return _keys[0]; } 
            else { _keys[0].first(); }
        }
        const K& midKey() const noexcept { 
            if constexpr (_isSet) { return _keys[keyCount() / 2]; } 
            else { _keys[keyCount() / 2].first(); }
        }
        const K& ithKey( const ssize_t& index ) const noexcept { 
            if constexpr (_isSet) { return _keys[index]; } 
            else { _keys[index].first(); }
        }
        SharedPtr<Node>& kthChild( const K& key ) { return _children[BSearchInChildren(key)]; }
        SharedPtr<Node>& ithChild( const ssize_t& index ) { return _children[index]; }
        const SharedPtr<Node>& kthChild( const K& key ) const { return _children[BSearchInChildren(key)]; }
        const SharedPtr<Node>& ithChild( const ssize_t& index ) const { return _children[index]; }
    public:
        ssize_t BSearchInKeys( const K& key ) const { // returns index in [0, 2 * Degree - 2] and ssize_t max in case key not found
            if (hasNoKeys()) { return -1; }
            ssize_t l = -1;
            ssize_t r = keyCount();
            ssize_t m = (r + l) / 2;

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
        ssize_t BSearchInChildren( const K& key ) const { // returns index in [0, Fanout - 1]
            if (hasNoKeys()) { return 0; }
            ssize_t l = 0; 
            ssize_t r = childCount();
            ssize_t m = (r + l) / 2;
            
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
        bool hasKey( const K& key ) const { return (BSearchInKeys(key) != -1); }
        bool hasInChildren( const K& key ) const {
            if (isLeaf()) {
                return hasKey(key);
            } else {
                auto childToCheck = kthChild(key);
                return childToCheck->hasInChildren(key);
            }
        }
    };

    SharedPtr<Node> _root;
private:
    struct constIterTraits {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = ssize_t;
        using value_type = V;
        using pointer    = const V*;
        using reference  = const V&;
    };
    struct nonConstIterTraits {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = ssize_t;
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
        BTreeIterator( SharedPtr<Node> node, const ssize_t index, bool atBegin = false, bool atEnd = false ) 
        : _root(node), _observed(node), _indexInNode(index), _atBegin(atBegin), _atEnd(atEnd) {}

        template <typename OtherTraits>
        BTreeIterator( const BTreeIterator<OtherTraits>& other ) 
        : _root( other._root ), _observed( other._observed ), _indexInNode( other._indexInNode )
        , _atEnd( other._atEnd ), _atBegin( other._atBegin ) {}
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
        bool isEnd() { return _atEnd; }
        bool isBegin() { return _atBegin; }
    public:
        BTreeIterator& goDownLeft() noexcept {
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
            _indexInNode = _observed->keyCount() - 1;
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
                if (_indexInNode < _observed->keyCount()) {
                    _indexInNode++;
                } else {
                    K maxKey = _observed->maxKey();
                    _observed = _observed->_parent;
                    while (_observed->keyCount() == _observed->BSearchInChildren( maxKey ) && _observed->_parent) {
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
        ssize_t _indexInNode;
        bool _atBegin;
        bool _atEnd;
    };
public:
    using iterT = BTreeIterator<
            std::conditional_t<_isSet,constIterTraits,nonConstIterTraits>
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
    BTree() {
        _root = makeShared<Node>();
    }

    BTree( const BTree& other ) = delete;
    BTree& operator=( const BTree& other ) = delete;
    BTree( BTree&& other ) = default;
    BTree& operator=( BTree&& other ) = default;

    ~BTree() = default;
public:
    iterT get( const K& key ) {
        return get( _root, key );
    }
    iterT get( SharedPtr<Node> node, const K& key ) {
        if ( node->hasKey(key) ) {
            return iterT( node, node->BSearchInKeys(key) );
        } else {
            if (node->hasInChildren(key)) {
                return get( node->kthChild(key), key );
            } else {
                return end();
            }
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
        return removeFromSubtree(_root, key);
    }
    bool contains( const K& key ) const {
        return _root->hasInChildren( key );
    }
    bool isEmpty() const {
        return _root;
    }
    const K& rightMostKey() {
        return rightMostKey(_root);
    }
    const K& leftMostKey() {
        return leftMostKey(_root);
    }
private:
    BTree& removeFromSubtree( SharedPtr<Node>& node, const K& key ) {
        if (node->isLeaf()) {
            return removeFromLeaf(node, key);
        } else {
            return removeFromNode(node, key);
        }
    }

    BTree& removeFromLeaf( SharedPtr<Node> node, const K& key ) {
        if (node->hasKey(key)) {
            if (!node->_parent) {
                node->_keys.removeAt(node->BSearchInKeys(key));
                return *this;
            } else {
                if (node->hasMinKeys()) {
                    ssize_t indexInParent = node->_parent->BSearchInChildren(key);
                    if (indexInParent > 0 && indexInParent < node->_parent->keyCount()) {
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
                    return *this;
                }
            }
        } else {
            return *this;
        }
    }

    BTree& removeFromNode( SharedPtr<Node>& node, const K& key ) {
        if (!node->hasKey(key)) {
            ssize_t index = node->BSearchInChildren(key);
            auto& child = node->ithChild(index);
            if (!child->hasMinKeys()) {
                return removeFromSubtree( child, key );
            } else {
                if (index > 0 && index < node->keyCount()) {
                    auto& left   = node->ithChild(index - 1);
                    auto& right  = node->ithChild(index + 1);
                    if (left->hasMinKeys() && right->hasMinKeys()) {
                        return merge(left, child)
                              .removeFromSubtree(
                            node->ithChild(index - 1), key
                                                 );
                    } else if (!right->hasMinKeys()) { 
                        return rotateRight(child)
                              .removeFromSubtree(
                            node->ithChild(index), key
                                                 );
                    } else {
                        return rotateLeft(child)
                              .removeFromSubtree(
                            node->ithChild(index), key 
                                                 );
                    }
                } else if (index == 0) {
                    auto& right  = node->ithChild(index + 1);
                    if (right->hasMinKeys()) {
                        return merge(child, right)
                              .removeFromSubtree(
                            node->ithChild(index), key
                                                 );
                    } else {
                        return rotateRight(child)
                              .removeFromSubtree(
                            node->ithChild(index), key
                                                 );
                    }
                } else {
                    auto& left  = node->ithChild(index - 1);
                    if (left->hasMinKeys()) {
                        return merge(left, child)
                              .removeFromSubtree(
                            node->ithChild(index - 1), key
                                                 );
                    } else {
                        return rotateLeft(child)
                              .removeFromSubtree(
                            node->ithChild(index), key
                                                 );
                    }
                }
            }
        } else { // node->hasKey(key)
            ssize_t index = node->BSearchInKeys(key);
            auto& predecessor = node->ithChild(index);
            auto& successor = node->ithChild(index + 1);
            if (predecessor->hasMinKeys() && successor->hasMinKeys()) {
                return merge( successor, predecessor )
                        .removeFromSubtree(
                            node->ithChild(index), key
                                           );
            } else if (!predecessor->hasMinKeys()) {
                auto& maxKey = rightMostKey( predecessor );
                node->_keys.removeAt(index);
                node->_keys.insertAt(maxKey, index);
                return removeFromSubtree(
                                predecessor, maxKey
                                         );
            } else {
                auto& minKey = leftMostKey( successor );
                node->_keys.removeAt(index);
                node->_keys.insertAt(minKey, index);
                return removeFromSubtree(
                                successor, minKey
                                         );
            }
        }
    } // removeFromNode()

    BTree& rotateLeft( SharedPtr<Node>& node ) {
        auto& parent = node->_parent;
        ssize_t indexInParent = parent->BSearchInChildren(node->minKey()) - 1;
        if (indexInParent >= 0) {
            auto& leftSibling = parent->ithChild(indexInParent);
            node->_keys.prepend(parent->ithKey(indexInParent));
            parent->_keys.removeAt(indexInParent);
            parent->_keys.insertAt(leftSibling->maxKey(), indexInParent);
            if (!node->isLeaf()) {
                node->_children.prepend(leftSibling->_children[leftSibling->childCount() - 1]);
                leftSibling->_children.removeAt(leftSibling->childCount() - 1);
            }
            leftSibling->_keys.removeAt(leftSibling->keyCount() - 1);
        }
        return *this;
    }   

    BTree& rotateRight( SharedPtr<Node>& node ) {
        auto& parent = node->_parent;
        ssize_t indexInParent = parent->BSearchInChildren(node->maxKey());
        if (indexInParent < parent->childCount() - 1) {
            auto rightSibling = parent->ithChild(indexInParent + 1);
            node->_keys.append(parent->ithKey(indexInParent));
            parent->_keys.removeAt(indexInParent);
            parent->_keys.insertAt(rightSibling->minKey(), indexInParent);
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
        ssize_t sepIndex = parent->BSearchInChildren( node1->maxKey() );
        K separator = parent->ithKey( sepIndex );

        parent->_keys.removeAt(sepIndex);
        node1->_keys.append(separator);
        node1->_keys.concat( node2->_keys );
        node1->_children.concat( node2->_children );
        parent->_children.removeAt(sepIndex + 1);
        if (!parent->_parent && parent->childCount() == 1) {
            node1->_parent = makeShared<Node>();
            _root = node1;
        }

        return *this;
    }
    
    BTree& split( SharedPtr<Node>& node ) {
        SharedPtr<Node> left, right;
        if (!node->_parent) {
            // node->_parent = makeShared<Node>();
            _root = node->_parent;
        }

        auto& parent = node->_parent;
        left->_parent = right->_parent = parent;
        left->_keys = node->_keys.subArray(0, node->keyCount()/2);
        right->_keys = node->_keys.subArray(node->keyCount()/2 + 1, node->keyCount());

        auto indexInParent = parent->BSearchInChildren(node->midKey());
        parent->_keys.insertAt(node->midKey(), indexInParent);
        parent->_children[indexInParent] = left;
        parent->_children.insertAt(right, indexInParent + 1); 
        return *this;
    }

    BTree& insertInSubtree( SharedPtr<Node>& root, const Pair<K,V>& pair ) {
        if (root->isLeaf()) {
            if (root->hasKey(pair.first())) {
                throw Exception( Exception::ErrorCode::KEY_COLLISION );
            } else {
                if constexpr (_isSet) {
                    root->_keys.insertAt(pair.first(), root->BSearchInChildren(pair.first()));
                } else {
                    root->_keys.insertAt(pair, root->BSearchInChildren(pair.first()));
                }
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

    const K& rightMostKey( SharedPtr<Node> node ) {
        while (!node->isLeaf()) {
            node = node->ithChild( node->childCount() - 1 );
        }
        return node->ithKey( node->keyCount() - 1 );
    }

    const K& leftMostKey( SharedPtr<Node> node ) {
        while (!node->isLeaf()) {
            node = node->ithChild( 0 );
        }
        return node->ithKey( 0 );
    }
};

#endif // BTREE_H