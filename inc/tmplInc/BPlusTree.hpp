#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "SharedPtr.hpp"
#include "ArraySequence.hpp"
#include "Pair.hpp"
#include "Ordering.hpp"
#include "Option.hpp"
// degree is a tree parameter defining the minimum and maximum amount of keys per node and leaf - [t-1; 2t-1] and children per node - [t; 2t]
// in that case fanout which is max amount of children per node equals degree * 2.
template <COrdered K, typename V, ssize_t Degree = 32>
class BPlusTree 
{
private:
    static constexpr bool _isSet = std::is_same_v<K,V>;
    static const size_t _fanout = Degree * 2;
    static const size_t _degree = Degree;
    struct Node 
    {
        WeakPtr<Node> _parent;
        // if node
        ArraySequence<K> _keys;
        ArraySequence<SharedPtr<Node>> _children;
        // if leaf
        WeakPtr<Node> _left;
        WeakPtr<Node> _right;
        using TContents = std::conditional_t<_isSet,V,Pair<K,V>>;
        ArraySequence<TContents> _contents;
    public:
        Node() = default;

        Node( const Node& other ) = default;
        Node& operator=( const Node& other ) = default;
        Node( Node&& other ) = default;
        Node& operator=( Node&& other ) = default;

        ~Node() = default;
    public:
        bool isLeaf() const noexcept { return _children.isEmpty(); }
        bool isFull() const noexcept {
            if (isLeaf()) { return _contents.getSize() == _fanout - 1; }
            else { return _keys.getSize() == _fanout - 1; }
         }
        bool hasNoKeys()  const noexcept { 
            if (isLeaf()) { return _contents.getSize() == 0; } 
            else { return _keys.getSize() == 0; }
        }
        bool hasMinKeys() const noexcept { 
            if (isLeaf()) { return _contents.getSize() == _degree - 1; }
            else { return _keys.getSize() == _degree - 1; }
        }
        bool canAddKey()  const noexcept { return _keys.getSize() < _fanout - 1; }

        ssize_t keyCount()   const noexcept { 
            if (isLeaf()) {
                return _contents.getSize();
            } else {
                return _keys.getSize(); 
            }
        }
        ssize_t childCount() const noexcept { return _children.getSize(); } 
        const K& maxKey() const noexcept { 
            if (isLeaf()) {
                if constexpr (_isSet) { return _contents[keyCount() - 1]; } 
                else { return _contents[keyCount() - 1].first(); }
            } else {
                return _keys[keyCount() - 1];
            }
        }
        const K& minKey() const noexcept { 
            if (isLeaf()) {
                if constexpr (_isSet) { return _contents[0]; } 
                else { return _contents[0].first(); }
            } else {
                return _keys[0];
            }
        }
        const K& midKey() const noexcept { 
            if (isLeaf()) {
                if constexpr (_isSet) { return _contents[keyCount() / 2]; } 
                else { return _contents[keyCount() / 2].first(); }
            } else {
                return _keys[keyCount() / 2];
            }
        }
        const K& ithKey( const ssize_t& index ) const noexcept { 
            if (isLeaf()) {
                if constexpr (_isSet) { return _contents[index]; } 
                else { return _contents[index].first(); }
            } else {
                return _keys[index];
            }
        }

        SharedPtr<Node>& kthChild( const K& key ) { return _children[BSearchInChildren(key)]; }
        SharedPtr<Node>& ithChild( const ssize_t& index ) { return _children[index]; }
        const SharedPtr<Node>& kthChild( const K& key ) const { return _children[BSearchInChildren(key)]; }
        const SharedPtr<Node>& ithChild( const ssize_t& index ) const { return _children[index]; }

        WeakPtr<Node>& parent() { return _parent; }
        WeakPtr<Node>& left() { return _left; }
        WeakPtr<Node>& right() { return _right; }
    public:
        ssize_t BSearchInChildren( const K& key ) const { // returns index [0, fanout - 1] in _children array so that ithChild(index) is the root of subtree containing that key
            if (hasNoKeys()) { return 0; } 
            ssize_t l = 0;
            ssize_t r = keyCount();
            ssize_t m = (r + l) / 2;

            K L = minKey();
            K R = maxKey();
            K M = ithKey(m);
            if (key < L) { return 0; }
            if (R <= key) { return r; }
            while (l < r - 1) {
                if (M <= key) {
                    l = m;
                } else {
                    r = m;
                }
                m = (r + l) / 2;
                M = ithKey(m);
            }
            return r;
        }
        ssize_t BSearchInContents( const K& key ) const { // returns index [0, fanout - 2] in _contents array so that _contents[index] is a pair such that pair.first() == key or -1 if search fails
            if (hasNoKeys()) { return -1; }
            ssize_t l = 0;
            ssize_t r = keyCount() - 1;
            ssize_t m = (r + l) / 2;

            K M = ithKey(m);
            K L = ithKey(l);
            K R = ithKey(r);
            if (L == key) { return l; }
            if (R == key) { return r; }
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
        bool hasInKeys( const K& key ) const noexcept { return (BSearchInContents(key) != -1); }
        bool hasInChildren( const K& key ) const noexcept { 
            if (isLeaf()) {
                return hasInKeys(key);
            } else {
                auto childToCheck = kthChild(key);
                return childToCheck->hasInChildren(key);
            }
        }
    };

    SharedPtr<Node> _root;
    ssize_t _size;
private:
    enum class iterState
    {
        atBegin = -1, 
        other   = 0,
        atEnd   = 1
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
    class BPlusTreeIterator
    {
    public:
        using iterator_category = typename IterTraits::iterator_category;
        using difference_type   = typename IterTraits::difference_type;
        using value_type = typename IterTraits::value_type;   
        using pointer    = typename IterTraits::pointer;   
        using reference  = typename IterTraits::reference; 
    public:
        BPlusTreeIterator() = default;
        BPlusTreeIterator( SharedPtr<Node> node, const ssize_t index, const int state ) 
        : _root(node), _observed(node), _indexInLeaf(index) {
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
        BPlusTreeIterator( const BPlusTreeIterator<OtherTraits>& other )
        : _observed( other._observed ), _indexInLeaf( other._indexInLeaf )
        , _root( other._root ), _state( other._state ) {}
    public: 
        reference operator*() noexcept {
            if constexpr(_isSet) {
                return _observed->_contents[_indexInLeaf];
            } else {
                return _observed->_contents[_indexInLeaf].second();
            }
        }
        pointer operator->() noexcept {
            return std::addressof(_observed->_contents[_indexInLeaf]);
        }

        BPlusTreeIterator& operator++() noexcept {
            return stepForward();
        }
        BPlusTreeIterator operator++(int) noexcept {
            auto res = *this;
            stepForward();
            return res;
        }
        BPlusTreeIterator& operator--() noexcept {
            return stepBack();
        }
        BPlusTreeIterator operator--(int) noexcept {
            auto res = *this;
            stepBack();
            return res;
        }

        friend bool operator==( const BPlusTreeIterator& lhs, const BPlusTreeIterator& rhs ) noexcept {
            return    lhs._observed == rhs._observed
                && lhs._indexInLeaf == rhs._indexInLeaf
                &&       lhs._state == rhs._state;
        }
        friend bool operator!=( const BPlusTreeIterator& lhs, const BPlusTreeIterator& rhs ) noexcept {
            return !(lhs == rhs);
        }
        bool isEnd()   const noexcept { return static_cast<int>(_state) == static_cast<int>(iterState::atEnd); }
        bool isBegin() const noexcept { return static_cast<int>(_state) == static_cast<int>(iterState::atBegin); }

        static BPlusTreeIterator begin( SharedPtr<Node> root ) noexcept {
            BPlusTreeIterator res( root, 0, -1);
            return res.setBegin().goDownLeft();
        }
        static BPlusTreeIterator end( SharedPtr<Node> root ) noexcept {
            BPlusTreeIterator res( root, 0, 1);
            res._observed = SharedPtr<Node>();
            return res;
        }
    private:
        BPlusTreeIterator& setEnd() noexcept {
            _state = iterState::atEnd;
            return *this;
        }
        BPlusTreeIterator& setBegin() noexcept {
            _state = iterState::atBegin;
            return *this;
        }
        BPlusTreeIterator& setMid() noexcept {
            _state = iterState::other;
            return *this;
        }

        BPlusTreeIterator& goDownLeft() noexcept {
            while (!_observed->isLeaf()) {
                _observed = _observed->ithChild(0);
            }
            _indexInLeaf = 0;
            return *this;
        }
        BPlusTreeIterator& goDownRight() noexcept {
            while (!_observed->isLeaf()) {
                _observed = _observed->ithChild( _observed->keyCount() );
            }
            _indexInLeaf = _observed->keyCount() - 1;
            return *this;
        }

        BPlusTreeIterator& stepForward() noexcept {
            if (isBegin()) { setMid(); }
            if ( isEnd() ) { return *this; }
            if (_indexInLeaf < _observed->keyCount() - 1) {
                _indexInLeaf++;
            } else {
                auto right = _observed->right();
                if (right) {
                    _observed = right.lock();
                    _indexInLeaf = 0;
                } else {
                    _observed = SharedPtr<Node>();
                    _indexInLeaf = 0;
                    setEnd();
                }
            }
            return *this;
        }
        BPlusTreeIterator& stepBack() noexcept {
            if ( isEnd() ) { setMid(); }
            if (isBegin()) { return *this; }
            if (_indexInLeaf > 0) {
                _indexInLeaf--;
            } else {
                auto left = _observed->left();
                if (left) {
                    _observed = left.lock();
                    _indexInLeaf = _observed->keyCount() - 1;
                } else {
                    _indexInLeaf = 0;
                    setBegin();
                }
            }
            return *this;
        }
    private:
        SharedPtr<Node> _root;
        SharedPtr<Node> _observed;
        ssize_t _indexInLeaf;
        iterState _state;
        template<class> friend class BPlusTreeIterator;
    };
public:
    using TIter = BPlusTreeIterator<
        std::conditional_t<_isSet,constIterTraits,nonConstIterTraits>
                                    >;
    using constTIter = BPlusTreeIterator<constIterTraits>;
    TIter begin() noexcept {
        return TIter::begin(_root);
    }
    constTIter begin() const noexcept {
        return constTIter::begin(_root);
    }
    TIter end() noexcept {
        return TIter::end(_root);
    }
    constTIter end() const noexcept {
        return constTIter::end(_root);
    }
public:
    BPlusTree() : _root( makeShared<Node>() ), _size(0) {}

    BPlusTree( const BPlusTree& other ) = delete;
    BPlusTree& operator=( const BPlusTree& other ) = delete;
    
    BPlusTree( BPlusTree&& other ) = default;
    BPlusTree& operator=( BPlusTree&& other ) = default;

    ~BPlusTree() = default;
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
    BPlusTree& insert( const V& value ) {
        return insertInSubtree( _root, Pair<V,V>( value, value ) );
    }
    BPlusTree& insert( const Pair<K,V>& pair ) {
        return insertInSubtree( _root, pair );
    }

    BPlusTree& remove( const K& key ) {
        return removeFromSubTree( _root, key );
    }

    bool contains( const K& key ) const {
        return _root->hasInChildren(key);
    }
    bool isEmpty() const {
        return _size == 0;
    }
    ssize_t getSize() const {
        return _size;
    }
private:
    TIter find( SharedPtr<Node> node, const K& key ) {
        if (node->hasInKeys(key)) {
            return TIter( node, node->BSearchInContents(key), 0);
        } else {
            if (node->isLeaf()) { return end(); }
            
            if (node->hasInChildren(key)) {
                return find( node->kthChild(key), key);
            } else {
                return end();
            }
        }
    }

    constTIter find( SharedPtr<Node> node, const K& key ) const {
        if (node->hasInKeys(key)) {
            return constTIter( node, node->BSearchInContents(key), 0);
        } else {
            if (node->isLeaf()) { return end(); }
            
            if (node->hasInChildren(key)) {
                return find( node->kthChild(key), key);
            } else {
                return end();
            }
        }
    }

    BPlusTree& insertInSubtree( SharedPtr<Node>& root, const Pair<K,V>& pair ) {
        auto parent = root->parent().lock();
        if (!parent) {
            if (root->isLeaf()) {
                if (root->hasInKeys( pair.first() )) {
                    throw Exception( Exception::ErrorCode::KEY_COLLISION );
                } else {
                    if (root->isFull()) {
                        return splitRoot()
                              .insertInSubtree( _root->kthChild(pair.first()), pair );
                    } else {
                        _size++;
                        if constexpr (_isSet) { root->_contents.insertAt( pair.first(), root->BSearchInChildren( pair.first() )); }
                        else { root->_contents.insertAt( pair, root->BSearchInChildren( pair.first() )); }
                        return *this;
                    }
                }
            } else {
                if (root->isFull()) {
                    return splitRoot()
                          .insertInSubtree( _root->kthChild( pair.first() ), pair );
                } else {
                    return insertInSubtree( _root->kthChild( pair.first() ), pair );
                }
            }
        } else {
            if (root->isLeaf()) {
                if (root->hasInKeys( pair.first() )) {
                    throw Exception( Exception::ErrorCode::KEY_COLLISION );
                } else {
                    if (root->isFull()) {
                        return split( parent, pair.first() )
                              .insertInSubtree( parent->kthChild( pair.first() ), pair );
                    } else {
                        _size++;
                        if constexpr (_isSet) { root->_contents.insertAt( pair.first(), root->BSearchInChildren( pair.first() )); }
                        else { root->_contents.insertAt( pair, root->BSearchInChildren( pair.first() )); }
                        return *this;
                    }
                }
            } else {
                if (root->isFull()) {
                    return split( parent, pair.first() )
                          .insertInSubtree( parent->kthChild( pair.first() ), pair );
                } else {
                    return insertInSubtree( root->kthChild( pair.first() ), pair );
                }
            }
        }
    }

    BPlusTree& splitRoot() {
        auto newRoot = makeShared<Node>();
        newRoot->_keys.append(_root->ithKey( _root->keyCount() / 2 ) );

        auto left  = makeShared<Node>();
        auto right = makeShared<Node>();

        left->parent() = right->parent() = newRoot;
        if (_root->isLeaf()) {
            left->_contents  = _root->_contents.subArray( 0, _root->keyCount() / 2 );
            right->_contents = _root->_contents.subArray( _root->keyCount() / 2, _root->keyCount() );  
            left->right() = right;
            right->left() = left;          
        } else {
            left->_keys = _root->_keys.subArray( 0, _root->keyCount() / 2 );
            right->_keys = _root->_keys.subArray( _root->keyCount() / 2 + 1, _root->keyCount() ); 

            left->_children  = _root->_children.subArray( 0, _root->childCount() / 2 );
            right->_children = _root->_children.subArray( _root->childCount() / 2, _root->childCount() );

            left->_children.map([&left]( SharedPtr<Node>& child) -> SharedPtr<Node> { child->parent() = left;
                                                                                      return child; });
                                                                                      
            right->_children.map([&right]( SharedPtr<Node>& child) -> SharedPtr<Node> { child->parent() = right;
                                                                                        return child; });
        }
        newRoot->_children.append(left);
        newRoot->_children.append(right);
        _root = newRoot;
        return *this;
    }

    BPlusTree& split( SharedPtr<Node>& parent, const K& key ) {
        size_t index = parent->BSearchInChildren(key);
        auto& node = parent->ithChild(index);
        auto right = makeShared<Node>();
        K separator = node->midKey();

        right->parent() = parent;

        if (!node->isLeaf()) {
            right->_children = node->_children.subArray( node->childCount() / 2, node->childCount() );
            right->_children.map([&right]( SharedPtr<Node>& child ) -> SharedPtr<Node> { child->parent() = right;
                                                                                         return child; });
            node->_children  = node->_children.subArray( 0, node->childCount() / 2 );

            right->_keys = node->_keys.subArray( node->keyCount() / 2 + 1, node->keyCount() );
            node->_keys  = node->_keys.subArray( 0, node->keyCount() / 2 );
        } else {
            right->_contents = node->_contents.subArray( node->keyCount() / 2, node->keyCount() );
            node->_contents  = node->_contents.subArray( 0, node->keyCount() / 2 );

            right->right() = node->right();
            node->right() = right;
            right->left() = node;
            if (right->right()) { right->right().lock()->left() = right; }
        }
        parent->_keys.insertAt( separator, index );
        parent->_children.insertAt( right, index + 1 );

        return *this;
    }

    BPlusTree& merge( SharedPtr<Node>& node1Ref, SharedPtr<Node>& node2Ref ) {
        SharedPtr<Node> node1 = node1Ref;
        SharedPtr<Node> node2 = node2Ref;
        
        auto parent = node1->parent().lock();
        auto index = parent->BSearchInChildren( node1->maxKey() );

        if (node1->isLeaf()) {
            node1->_contents.concat( node2->_contents );
            
            node1->right() = node2->right();
            if (node1->right()) { node1->right().lock()->left() = node1; }
        } else {
            node1->_keys.append( parent->ithKey(index) );
            node1->_keys.concat( node2->_keys );

            node1->_children.concat( node2->_children );
            node1->_children.map([&node1]( SharedPtr<Node>& child ) -> SharedPtr<Node> { child->parent() = node1;
                                                                                         return child; });            
        }

        parent->_keys.removeAt( index );
        parent->_children.removeAt( index + 1 );

        if (!parent->parent() && parent->keyCount() == 0) {
            node1->parent() = WeakPtr<Node>();
            _root = node1;
        }
        return *this;
    }

    BPlusTree& rotateRight( SharedPtr<Node>& node ) {
        auto parent = node->parent().lock();
        auto index  = parent->BSearchInChildren(node->maxKey()) + 1;
        auto& right = parent->ithChild(index);

        if (node->isLeaf()) {
            node->_contents.append( right->_contents[0] );
            right->_contents.removeAt(0);
            
            if constexpr (_isSet) { parent->_keys.setAt( right->_contents[0], index - 1 ); }
            else { parent->_keys.setAt( right->_contents[0].first(), index - 1 ); }
        } else {
            node->_keys.append( right->ithKey(0) );
            right->_keys.removeAt(0);

            node->_children.append( right->ithChild(0) );
            node->ithChild( node->childCount() - 1 )->parent() = node;
            right->_children.removeAt(0);

            parent->_keys.setAt( right->ithKey(0), index );
        }
        return *this;
    }

    BPlusTree& rotateLeft( SharedPtr<Node>& node ) {
        auto parent = node->parent().lock();
        auto index  = parent->BSearchInChildren(node->maxKey()) - 1;
        auto& left = parent->ithChild(index);

        if (node->isLeaf()) {
            node->_contents.prepend( left->_contents[left->keyCount() - 1] );
            left->_contents.removeAt(left->keyCount() - 1);
            
            if constexpr (_isSet) { parent->_keys.setAt( node->_contents[0], index ); }
            else { parent->_keys.setAt( node->_contents[0].first(), index ); }
        } else {
            node->_keys.prepend( left->ithKey(left->keyCount() - 1) );
            left->_keys.removeAt( left->keyCount() - 1 );

            node->_children.prepend( left->ithChild(left->childCount() - 1) );
            node->ithChild(0)->parent() = node;
            left->_children.removeAt(left->childCount() - 1);

            parent->_keys.setAt( node->ithKey(0), index );
        }

        return *this;
    }

    BPlusTree& removeFromSubTree( SharedPtr<Node>& node, const K& key ) {
        if (node->isLeaf()) {
            return removeFromLeaf( node, key );
        } else {
            return removeFromNode( node, key );
        }
    }

    BPlusTree& removeFromNode( SharedPtr<Node>& node, const K& key ) {
        auto& child = node->kthChild(key);
        auto  index = node->BSearchInChildren(key);
        if (child->hasMinKeys()) {
            if (index > 0 && index < node->keyCount()) {
                auto& left  = node->ithChild(index - 1);
                auto& right = node->ithChild(index + 1);
                if (left->hasMinKeys() && right->hasMinKeys()) {
                    return merge( left, child )
                          .removeFromSubTree(
                                node->kthChild(key), key
                                                );
                } else if (!left->hasMinKeys()) {
                    return rotateLeft( child )
                          .removeFromSubTree(
                                node->ithChild(index), key 
                                                );
                } else {
                    return rotateRight( child )
                          .removeFromSubTree(
                                node->ithChild(index), key
                                                );
                }
            } else if (index == 0) {
                auto& right = node->ithChild(index + 1);
                if (right->hasMinKeys()) {
                    bool isRoot = !node->parent();
                    return merge( child, right )
                          .removeFromSubTree(
                ( isRoot ? _root : node->kthChild(key) ), key
                                                );
                } else {
                    return rotateRight( child )
                          .removeFromSubTree(
                                node->ithChild(index), key
                                                );
                }
            } else {
                auto& left  = node->ithChild(index - 1);
                if (left->hasMinKeys() && left->hasMinKeys()) {
                    bool isRoot = !node->parent();
                    return merge( left, child )
                          .removeFromSubTree(
                ( isRoot ? _root : node->kthChild(key) ), key
                                                );
                } else {
                    return rotateLeft( child )
                          .removeFromSubTree(
                                node->ithChild(index), key 
                                                );
                }
            }
        } else {
            return removeFromSubTree( child, key );
        }
    }

    BPlusTree& removeFromLeaf( SharedPtr<Node>& leaf, const K& key ) {
        auto parent = leaf->parent().lock();
        if (leaf->hasInKeys(key)) {
            _size--;
            if (!parent) {
                leaf->_contents.removeAt( leaf->BSearchInContents(key) );
                return *this;
            } else {
                if (leaf->hasMinKeys()) {
                    auto left  = leaf->left().lock();
                    auto right = leaf->right().lock();
                    leaf->_contents.removeAt( leaf->BSearchInContents(key) );
                    if (left && right) {
                        if (left->hasMinKeys() && leaf->hasMinKeys()) { return merge(left, leaf); }
                        else if (!left->hasMinKeys()) { return rotateLeft(leaf); }
                        else                          { return rotateRight(leaf); }
                    } else if (left) {
                        if (left->hasMinKeys()) { return merge( left, leaf ); } 
                        else                    { return rotateLeft( leaf ); }
                    } else {
                        if (right->hasMinKeys()) { return merge( leaf, right ); }
                        else                     { return rotateRight(leaf); }
                    }
                } else {
                    leaf->_contents.removeAt( leaf->BSearchInContents(key) );
                    return *this;
                }
            }
        }
        return *this;
    }
};

#endif // BPLUSTREE_H