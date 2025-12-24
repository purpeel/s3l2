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
    static constexpr ssize_t _fanout = Degree * 2;
    static constexpr ssize_t _degree = Degree;
    struct Node 
    {
        SharedPtr<Node> _parent;
        SharedPtr<Node> _left;
        SharedPtr<Node> _right;
        ArraySequence<K> _keys;
        ArraySequence<SharedPtr<Node>> _children;

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
        bool isFull() const noexcept { return _children.getSize() == _fanout; }
        bool hasNoKeys()  const noexcept { return _keys.getSize() == 0; }
        bool hasMinKeys() const noexcept { return _keys.getSize() == _degree - 1; }
        bool canAddKey()  const noexcept { return _keys.getSize() < _fanout - 1; }

        ssize_t keyCount()   const noexcept { return _keys.getSixe(); }
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

        SharedPtr<Node>& kthChild( const K& key ) { return _children[BSearchInChildren(key)]; }
        SharedPtr<Node>& ithChild( const ssize_t& index ) { return _children[index]; }
        const SharedPtr<Node>& kthChild( const K& key ) const { return _children[BSearchInChildren(key)]; }
        const SharedPtr<Node>& ithChild( const ssize_t& index ) const { return _children[index]; }
    public:
        ssize_t BSearchInChildren( const K& key ) const { // returns index [0, fanout - 1] in _children array so that ithChild(index) is the root of subtree containing that key
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
        bool hasInKeys( const K& key ) const noexcept { return (BSearchInContents() != -1); }
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
        reference operator->() const noexcept {
            if constexpr(_isSet) {
                return _observed->_keys[_indexInLeaf];
            } else {
                return _observed->_keys[_indexInLeaf].second();
            }
        }
        pointer operator*() const noexcept {
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
            return res.setEnd().goDownRight();
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
            if (_indexInLeaf < _observed->keyCount()) {
                _indexInLeaf++;
            } else {
                if (_observed->_right) {
                    _observed = _observed->_right;
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
                if (_observed->_left) {
                    _observed = _observed->_left;
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
        enum class iterState
        {
            atBegin = -1, 
            other   = 0,
            atEnd   = 1
        };
        iterState _state;
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
    BPlusTree() {
        _root = makeShared<Node>();
    }

    BPlusTree( const BPlusTree& other ) = delete;
    BPlusTree& operator=( const BPlusTree& other ) = delete;
    
    BPlusTree( BPlusTree&& other ) = default;
    BPlusTree& operator=( BPlusTree&& other ) = default;

    ~BPlusTree() = default;
public:
    template <bool isSet = _isSet> requires(!isSet)
    V& get( const K& key );
    const V& get( const V& key ) const;

    TIter find( const K& key );
    constTIter find( const K& key ) const;
    
    template <bool isSet = _isSet> requires(isSet)
    BPlusTree& insert( const V& value );
    BPlusTree& insert( const Pair<K,V>& pair );

    BPlusTree& remove( const K& key );

    bool contains( const K& key ) const;
    bool isEmpty() const;
    ssize_t getSize() const;
private:
    
};

#endif // BPLUSTREE_H