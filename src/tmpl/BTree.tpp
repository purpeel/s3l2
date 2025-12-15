template <typename K, typename T>
BTree<K,T>::BTreeNode::BTreeNode()
: _isLeaf( false ), _parent( nullptr )
, _values( ArraySequence<Pair<K,T>>() )
, _nodes( ArraySequence<BTreeNode*>() ) {}

template <typename K, typename T>
BTree<K,T>::BTreeNode::BTreeNode( SharedPtr<BTreeNode> parent )
: _isLeaf( true ), _parent( parent )
, _values( ArraySequence<Pair<K,T>>() )
, _nodes( ArraySequence<BTreeNode*>() ) {}

template <typename K, typename T>
BTree<K,T>::BTreeNode::BTreeNode( const BTreeNode& other )
: _isLeaf( other._isLeaf ), _parent( other._parent )
, _values( other._values ), _nodes( other._nodes ) {}

template <typename K, typename T>
BTree<K,T>::BTreeNode& BTree<K,T>::BTreeNode::operator=( const BTreeNode& other ) {
    if ( this != &other ) {
        _isLeaf = other._isLeaf;
        _parent = other._parent;
        _values = other._values;
        _nodes  = other._nodes;
    }
    return *this;
}

template <typename K, typename T>
BTree<K,T>::BTreeNode::BTreeNode( BTreeNode&& other )
: _isLeaf( other._isLeaf ), _parent( other._parent )
, _values( std::move(other._values) ), _nodes( std::move(other._nodes) ) {
    other._isLeaf = false;
    other._parent = nullptr;
}

template <typename K, typename T>
BTree<K,T>::BTreeNode& BTree<K,T>::BTreeNode::operator=( BTreeNode&& other ) {
    if ( this != &other ) {
        _isLeaf = other._isLeaf;
        _parent = other._parent;
        _values = std::move(other._values);
        _nodes  = std::move(other._nodes);

        other._isLeaf = false;
        other._parent = nullptr;    
    }
    return *this;
}

template <typename K, typename T>
BTree<K,T>::BTree() 
: _minDegree( 4 )
, _root( makeShared<Node>() ) {}

template <typename K, typename T>
BTree<K,T>::BTree( const size_t& degree )
: _minDegree( degree )
, _root( makeShared<Node>() ) {}

template <typename K, typename T>
BTree<K,T>::BTree( const BTree<K,T>& other ) 
: _minDegree( other._minDegree )
, _root( other._root ) {} 

template <typename K, typename T>
BTree<K,T>& BTree<K,T>::operator=( const BTree<K,T>& other ) {
    if ( this != & other ) {
        _minDegree = other._minDegree;
        _root = other._root;
    }
    return *this;
}

template <typename K, typename T>
BTree<K,T>::BTree( BTree<K,T>&& other ) 
: _minDegree( std::move(other._minDegree) )
, _root( std::move(other._root) ) {} 

template <typename K, typename T>
BTree<K,T>& BTree<K,T>::operator=( BTree<K,T>&& other ) {
    if ( this != & other ) {
        _minDegree = std::move(other._minDegree);
        _root = std::move(other._root);
    }
    return *this;
}

