#ifndef BTREE_H
#define BTREE_H

#include "SharedPtr.hpp"
#include "WeakPtr.hpp"
#include "ArraySequence.hpp"
#include "Pair.hpp"
#include "Ordering.hpp"
// degree is a tree parameter defining the minimum and maximum amount of keys per node - [t-1; 2t-1] and children per node - [t; 2t]
// in that case fanout which is max amount of children per node equals degree * 2.
template <COrdered K, typename V, ssize_t Degree = 32>
class BTree
{
private:
    static constexpr bool _isSet = std::is_same_v<K,V>;
    static const size_t _fanout = Degree * 2;
    static const size_t _degree = Degree;
    struct Node {
        WeakPtr<Node> _parent;

        using TKeys = std::conditional_t<_isSet, V, Pair<K,V>>;
        ArraySequence<TKeys> _keys;
        ArraySequence<SharedPtr<Node>> _children;
    public:
        Node() = default;

        Node( const Node& other ) = default;
        Node& operator=( const Node& other ) = default;
        Node( Node&& other ) = delete;
        Node& operator=( Node&& other ) = delete;

        ~Node() = default;
    public:
        bool isLeaf() const noexcept { return _children.isEmpty(); }
        bool isFull() const noexcept { return _keys.getSize() == 2 * _degree - 1; }
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
            else { return _keys[0].first(); }
        }
        const K& midKey() const noexcept { 
            if constexpr (_isSet) { return _keys[keyCount() / 2]; } 
            else { return _keys[keyCount() / 2].first(); }
        }
        const K& ithKey( const ssize_t& index ) const noexcept { 
            if constexpr (_isSet) { return _keys[index]; } 
            else { return _keys[index].first(); }
        }

        const TKeys& minContent() const noexcept {
            if constexpr (_isSet) { return _keys[0]; }
            else { return _keys[0]; }
        }
        const TKeys& maxContent() const noexcept {
            if constexpr (_isSet) { return _keys[keyCount() - 1]; }
            else { return _keys[keyCount() - 1]; }
        }
        const TKeys& midContent() const noexcept {
            if constexpr (_isSet) { return _keys[keyCount()/2]; }
            else { return _keys[keyCount()/2]; }
        }
        const TKeys& ithContent( const ssize_t& index ) const noexcept {
            if constexpr (_isSet) { return _keys[index]; }
            else { return _keys[index]; }
        }

        WeakPtr<Node>& parent() { return _parent; }
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
            ssize_t r = keyCount();
            ssize_t m = (r + l) / 2;
            
            K L = minKey();
            K R = maxKey();
            K M = ithKey(m);

            if (key < L) { return 0; }
            if (R < key) { return r; } 

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
                if (!hasKey(key)) {
                    auto childToCheck = kthChild(key);
                    return childToCheck->hasInChildren(key);
                } else {
                    return true;
                }
            }
        }
    };

    SharedPtr<Node> _root;
    ssize_t _size;
private:
    enum class iterState 
    {
        atBegin = -1,
        other = 0,
        atEnd = 1
    };
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
    class BTreeIterator {
    public:
        using iterator_category = typename IterTraits::iterator_category;
        using difference_type   = typename IterTraits::difference_type;
        using value_type = typename IterTraits::value_type;   
        using pointer    = typename IterTraits::pointer;   
        using reference  = typename IterTraits::reference; 
    public:
        BTreeIterator() = default;
        BTreeIterator( SharedPtr<Node> node, const ssize_t index, const int state ) 
        : _root(node), _observed(node), _indexInNode(index) {
            switch(state)
            {
            case -1:
                setBegin();
                break;
            case 0:
                setMid();
                break;
            case 1:
                setEnd();
                break;
            default:
                throw Exception( Exception::ErrorCode::INVALID_ITERATOR );
            } 
        }

        template <typename OtherTraits>
        BTreeIterator( const BTreeIterator<OtherTraits>& other ) 
        : _root( other._root ), _observed( other._observed ), _indexInNode( other._indexInNode )
        , _state( other._state ) {}
    public:
        reference operator*() const noexcept {
            if constexpr(_isSet) {
                return _observed->_keys[_indexInNode];
            } else {
                return _observed->_keys[_indexInNode].second();
            }
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
            return    lhs._observed == rhs._observed 
                && lhs._indexInNode == rhs._indexInNode 
                &&       lhs._state == rhs._state;
        }
        friend bool operator!=( const BTreeIterator& lhs, const BTreeIterator& rhs ) noexcept { 
            return !( lhs == rhs );
        }
        bool isEnd()   const noexcept { return static_cast<int>(_state) == static_cast<int>(iterState::atEnd); }
        bool isBegin() const noexcept { return static_cast<int>(_state) == static_cast<int>(iterState::atBegin); }

        static BTreeIterator begin( SharedPtr<Node> root ) noexcept {
            BTreeIterator res( root, 0, -1 );
            return res.goDownLeft().setBegin();
        }
        static BTreeIterator end( SharedPtr<Node> root ) noexcept {
            BTreeIterator res( root, 0, 1 );
            res.observed() = SharedPtr<Node>();
            return res;
        }
    private:
        BTreeIterator& setEnd() noexcept { 
            _state = iterState::atEnd;
            return *this;
        }
        BTreeIterator& setBegin() noexcept { 
            _state = iterState::atBegin;
            return *this;
        }
        BTreeIterator& setMid() noexcept { 
            _state = iterState::other;
            return *this;
        }
        BTreeIterator& goDownLeft() noexcept {
            while (!_observed->isLeaf()) {
                _observed = _observed->ithChild(0);
            }
            _indexInNode = 0;
            return *this;
        }

        BTreeIterator& goDownRight() noexcept {
            while (!_observed->isLeaf()) {
                _observed = _observed->ithChild( _observed->keyCount() );
            }
            _indexInNode = _observed->keyCount() - 1;

            if (_indexInNode == 0 && _observed->parent().lock()->BSearchInChildren( _observed->minKey() ) == 0) {
                return setBegin();
            }

            return *this;
        }

        BTreeIterator& goUp() noexcept {
            while (_observed->parent()) {
                _observed = _observed->parent().lock();
            }
            return *this;
        }

        BTreeIterator& stepForward() noexcept {
            if (isEnd()) { return *this; }
            if (isBegin()) { setMid(); }
            if (_observed->isLeaf()) {
                if (_indexInNode < _observed->keyCount() - 1) {
                    _indexInNode++;
                } else {
                    K maxKey = _observed->maxKey();
                    _observed = _observed->parent();
                    if (_observed) {
                        while (_observed->keyCount() == _observed->BSearchInChildren( maxKey ) && _observed->_parent) {
                            _observed = _observed->parent().lock();
                        }
                        if (_observed->BSearchInChildren( maxKey ) == _observed->keyCount()) {
                            setEnd();
                            _observed = SharedPtr<Node>();
                            _indexInNode = 0;
                        } else {
                            _indexInNode = _observed->BSearchInChildren( maxKey );
                        }
                    } else {
                        setEnd();
                        _indexInNode = 0;
                    }
                }
            } else {
                _observed = _observed->ithChild(_indexInNode + 1);
                return goDownLeft();
            }
            return *this;
        }

        BTreeIterator& stepBack() noexcept {
            if (isBegin()) { return *this; }
            if (isEnd()) {
                setMid();
                _observed = _root;
                return goDownRight(); 
            }
            if (_observed->isLeaf()) {
                if (_indexInNode > 0) {
                    _indexInNode--;
                    if (_indexInNode == 0 && !_observed->_parent) { return setBegin(); }
                } else {
                    auto initialLeaf = _observed;       
                    K minKey = _observed->minKey();
                    _observed = _observed->parent();
                    if (_observed) {
                        while (_observed->BSearchInChildren( minKey ) == 0 && _observed->_parent) {
                            _observed = _observed->parent().lock();
                        }
                        if (_observed->BSearchInChildren( minKey ) == 0) { 
                            setBegin();
                            _observed = initialLeaf;
                            _indexInNode = 0;
                        } else {
                            _indexInNode = _observed->BSearchInChildren( minKey ) - 1;
                        }
                    } else {
                        setBegin();
                        _observed = initialLeaf;
                        _indexInNode = 0;
                    }
                }
            } else {
                _observed = _observed->ithChild(_indexInNode);
                return goDownRight();
            }
            return *this;
        }

        SharedPtr<Node>& observed() { return _observed; }
    private:
        SharedPtr<Node> _root;
        SharedPtr<Node> _observed;
        ssize_t _indexInNode;
        iterState _state;
        template<class> friend class BTreeIterator;
    };
public:
    using TIter = BTreeIterator<
            std::conditional_t<_isSet,constIterTraits,nonConstIterTraits>
                                >;
    using constTIter = BTreeIterator<constIterTraits>;
    TIter begin() noexcept {
        return TIter::begin(_root);
    }
    constTIter begin() const noexcept { //TODO ifEmpty() -> return end();
        return constTIter::begin(_root);
    }
    TIter end() noexcept {
        return TIter::end(_root);
    }
    constTIter end() const noexcept {
        return constTIter::end(_root);
    }
public:
    BTree() : _root( makeShared<Node>() ) {}

    BTree( const BTree& other ) = delete;
    BTree& operator=( const BTree& other ) = delete;
    BTree( BTree&& other ) = default;
    BTree& operator=( BTree&& other ) = default;

    ~BTree() = default;
public:
    template <bool isSet = _isSet> requires(!isSet)  
    V& get( const K& key ) {
        auto it = find(key);
        if (it != end()) { return *it; } 
        else { throw Exception( Exception::ErrorCode::ABSENT_KEY ); }
    }
    const V& get( const K& key ) const {
        auto it = find(key);
        if (it != end()) { return *it; } 
        else { throw Exception( Exception::ErrorCode::ABSENT_KEY ); }
    }
    TIter find( const K& key ) {
        return find( _root, key );
    }
    constTIter find( const K& key ) const {
        return find( _root, key );
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
    ssize_t getSize() const {
        return _size;
    }
    const K& rightMostKey() const {
        return rightMostKey(_root);
    }
    const K& leftMostKey() const {
        return leftMostKey(_root);
    }
private:
    TIter find( SharedPtr<Node> node, const K& key ) {
        if ( node->hasKey(key) ) {
            return TIter( node, node->BSearchInKeys(key), 0 );
        } else {
            if (node->hasInChildren(key)) {
                return find( node->kthChild(key), key );
            } else {
                return end();
            }
        }
    }
    constTIter find( SharedPtr<Node> node, const K& key ) const {
        if ( node->hasKey(key) ) {
            return TIter( node, node->BSearchInKeys(key), 0);
        } else {
            if (node->isLeaf()) { return end(); }
            if (node->hasInChildren(key)) {
                return find( node->kthChild(key), key );
            } else {
                return end();
            }
        }
    }
    BTree& removeFromSubtree( SharedPtr<Node>& node, const K& key ) {
        if (node->isLeaf()) {
            return removeFromLeaf(node, key);
        } else {
            return removeFromNode(node, key);
        }
    }

    BTree& removeFromLeaf( SharedPtr<Node> node, const K& key ) {
        auto parent = node->parent().lock();
        if (node->hasKey(key)) {
            if (!parent) {
                node->_keys.removeAt(node->BSearchInKeys(key));
                return *this;
            } else {
                if (node->hasMinKeys()) {
                    ssize_t index = parent->BSearchInChildren(key);
                    node->_keys.removeAt(node->BSearchInKeys(key));

                    if (index > 0 && index < parent->keyCount()) {
                        auto& left   = parent->ithChild(index - 1);
                        auto& right  = parent->ithChild(index + 1);
                        if (left->hasMinKeys() && right->hasMinKeys()) { return merge( left, node ); } 
                        else if (!left->hasMinKeys())  { return  rotateLeft( node ); } 
                        else                           { return rotateRight( node ); }
                    } else if (index == 0) {
                        auto& right  = parent->ithChild(index + 1);
                        if (right->hasMinKeys()) { return merge( node, right ); } 
                        else                     { return rotateRight( node ); }
                    } else {
                        auto& left  = parent->ithChild(index - 1);
                        if (left->hasMinKeys()) { return merge( left, node ); } 
                        else                    { return rotateLeft( node ); }
                    }
                } else {
                    node->_keys.removeAt( node->BSearchInKeys(key) );
                    return *this;
                }
            }
        }
        return *this;
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
                            node->kthChild(key), key
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
                        bool isRoot = !node->parent(); // if two last children of a root are merged, they become a new root
                        return merge(child, right)
                              .removeFromSubtree(
                    (isRoot ? _root : node->kthChild(key) ), key
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
                        bool isRoot = !node->parent();
                        return merge(left, child)
                              .removeFromSubtree(
                    (isRoot ? _root : node->kthChild(key) ), key
                                                 );
                    } else {
                        return rotateLeft(child)
                              .removeFromSubtree(
                            node->ithChild(index), key
                                                 );
                    }
                }
            }
        } else { // if !node->hasKey(key)
            ssize_t index = node->BSearchInKeys(key);
            auto& predecessor = node->ithChild(index);
            auto& successor = node->ithChild(index + 1);
            if (predecessor->hasMinKeys() && successor->hasMinKeys()) {
                bool isRoot = !node->parent();
                return merge( predecessor, successor )
                      .removeFromSubtree(
        (isRoot ? _root : node->ithChild( node->BSearchInChildren(key) ) ), key
                                         );
            } else if (!predecessor->hasMinKeys()) {
                auto& maxKey = rightMostKey( predecessor );
                node->_keys.setAt(maxKey, index);
                return removeFromSubtree(
                                predecessor, maxKey
                                         );
            } else {
                auto& minKey = leftMostKey( successor );
                node->_keys.setAt(minKey, index);
                return removeFromSubtree(
                                successor, minKey
                                         );
            }
        }
    } // removeFromNode()

    BTree& rotateLeft( SharedPtr<Node>& node ) {
        auto parent = node->parent().lock();
        ssize_t index = parent->BSearchInChildren(node->minKey()) - 1;
        
        auto& leftSibling = parent->ithChild(index);
        node->_keys.prepend( parent->ithKey(index) );
        parent->_keys.setAt( leftSibling->maxKey(), index );

        if (!node->isLeaf()) {
            node->_children.prepend( leftSibling->ithChild(leftSibling->childCount() - 1) );
            leftSibling->_children.removeAt( leftSibling->childCount() - 1  );
            node->ithChild( 0 )->parent() = node;
        }
        leftSibling->_keys.removeAt(leftSibling->keyCount() - 1);
        return *this;
    }   

    BTree& rotateRight( SharedPtr<Node>& node ) {
        auto parent = node->parent().lock();
        auto index = parent->BSearchInChildren(node->maxKey());

        auto& rightSibling = parent->ithChild(index + 1);
        node->_keys.append(parent->ithKey(index));
        parent->_keys.setAt( rightSibling->minKey(), index);

        if (!node->isLeaf()) {
            node->_children.append(rightSibling->ithChild(0));
            rightSibling->_children.removeAt(0);
            node->ithChild( node->childCount() - 1 )->parent() = node;
        }
        rightSibling->_keys.removeAt(0);
        return *this;
    }    
    
    BTree& merge( SharedPtr<Node>& node1, SharedPtr<Node>& node2 ) {
        auto parent = node1->parent().lock();
        ssize_t sepIndex = parent->BSearchInChildren( node1->maxKey() );
        K separator = parent->ithKey( sepIndex );

        node1->_keys.append(separator);
        node1->_keys.concat( node2->_keys );
        node1->_children.concat( node2->_children );

        if (!node1->isLeaf()) {
            node1->_children.map([&node1]( SharedPtr<Node>& child ) -> SharedPtr<Node> { child->parent() = node1;
                                                                                         return child; } );
        }

        parent->_keys.removeAt(sepIndex);
        parent->_children.removeAt(sepIndex + 1);
        
        if (!parent->parent() && parent->keyCount() == 0) {
            node1->parent() = makeShared<Node>();
            _root = node1;
        }       

        return *this;
    }

    BTree& splitRoot() {
        auto newRoot = makeShared<Node>();
        newRoot->_keys.append( _root->midContent() );

        auto left  = makeShared<Node>();
        auto right = makeShared<Node>();

        left->_keys  = _root->_keys.subArray( 0, _root->keyCount() / 2 );
        right->_keys = _root->_keys.subArray( _root->keyCount() / 2 + 1, _root->keyCount() );
        left->parent()  = newRoot;
        right->parent() = newRoot;

        if (!_root->isLeaf()) {
            left->_children  = _root->_children.subArray( 0, _root->keyCount() / 2 + 1 );
            left->_children.map([&left]( SharedPtr<Node>& child ) -> SharedPtr<Node> { child->parent() = left;
                                                                                       return child; } );
            right->_children = _root->_children.subArray( _root->keyCount() / 2 + 1, _root->keyCount() + 1 );
            right->_children.map([&right]( SharedPtr<Node>& child ) -> SharedPtr<Node> { child->parent() = right;
                                                                                         return child; } );
        }
        newRoot->_children.append(left);
        newRoot->_children.append(right);
        _root = newRoot;
        return *this;
    }

    BTree& split( SharedPtr<Node>& parent, const K& key ) {
        ssize_t indexInParent = parent->BSearchInChildren(key);
        auto& node = parent->kthChild(key);
        parent->_keys.insertAt( node->midContent(), indexInParent );

        auto left  = makeShared<Node>();
        auto right = makeShared<Node>();
        
        left->_keys      = node->_keys.subArray( 0, node->keyCount()/2 );
        right->_keys     = node->_keys.subArray( node->keyCount()/2 + 1, node->keyCount() );
        left->parent()   = parent;
        right->parent()  = parent;                            

        if (!node->isLeaf()) {
            left->_children  = node->_children.subArray( 0, node->keyCount() / 2 + 1 );
            left->_children.map([&left]( SharedPtr<Node>& child ) -> SharedPtr<Node> { child->_parent = left;
                                                                                       return child; } );
            right->_children = node->_children.subArray( node->keyCount() / 2 + 1, node->keyCount() + 1);
            right->_children.map([&right]( SharedPtr<Node>& child ) -> SharedPtr<Node> { child->_parent = right;
                                                                                         return child; } );
        }
        parent->_children[indexInParent] = left;
        parent->_children.insertAt(right, indexInParent + 1);
        return *this;
    }

    BTree& insertInSubtree( SharedPtr<Node>& root, const Pair<K,V>& pair ) {
        auto parent = root->parent().lock();
        if (!parent) {
            if (root->isLeaf()) {
                if (root->hasKey(pair.first())) {
                    throw Exception( Exception::ErrorCode::KEY_COLLISION );
                } else {
                    if (root->isFull()) {
                        return splitRoot()
                              .insertInSubtree( _root->kthChild( pair.first() ), pair );
                    } else {
                        if constexpr(_isSet) { root->_keys.insertAt( pair.first(), root->BSearchInChildren(pair.first()) ); } 
                        else { root->_keys.insertAt( pair, _root->BSearchInChildren(pair.first()) ); }
                        return *this;
                    }
                }
            } else {
                if (root->isFull()) {
                    return splitRoot()
                          .insertInSubtree( _root->kthChild( pair.first() ) , pair );
                } else {
                    return insertInSubtree( _root->kthChild( pair.first() ) , pair );
                }
            }
        } else {
            if (root->isLeaf()) {
                if (root->hasKey(pair.first())) {
                    throw Exception( Exception::ErrorCode::KEY_COLLISION );
                } else {
                    if (root->isFull()) {
                        return split( parent, pair.first())
                              .insertInSubtree( parent->kthChild(pair.first()), pair );
                    } else {
                        if constexpr(_isSet) { root->_keys.insertAt( pair.first(), root->BSearchInChildren(pair.first()) ); } 
                        else { root->_keys.insertAt( pair, root->BSearchInChildren(pair.first()) ); }
                        return *this;
                    }
                }
            } else {
                if (root->isFull()) {
                    return split( parent, pair.first() )
                          .insertInSubtree( parent->kthChild( pair.first() ) , pair );
                } else {
                    return insertInSubtree( root->kthChild( pair.first() ) , pair );
                }
            }
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