#ifndef BTREE_H
#define BTREE_H

#include <string>
#include "SharedPtr.hpp"
#include "ArraySequence.hpp"
#include "Pair.hpp"

template <typename K, typename T>
class BTree
{
private:
    struct BTreeNode {
        bool _isLeaf;
        SharedPtr<BTreeNode> _parent;

        ArraySequence<Pair<K,T>> _values;
        ArraySequence<BTreeNode*> _nodes;  

        BTreeNode();
        BTreeNode( SharedPtr<BTreeNode> parent );

        BTreeNode( const BTreeNode& other );
        BTreeNode& operator=( const BTreeNode& other );

        BTreeNode( BTreeNode&& other );
        BTreeNode& operator=( BTreeNode&& other );

        ~BTreeNode() = default;
    };
    using Node = BTreeNode<K,T>;
public:
    BTree();

    BTree( const size_t& degree );

    BTree( const BTree<K,T>& other );
    BTree<K,T>& operator=( const BTree<K,T>& other );
    
    BTree( BTree<K,T>&& other );
    BTree<K,T>& operator=( BTree<K,T>&& other );

    ~BTree() = default;
public:
    T& get( K key );
    const T& get( K key ) const;
    void insert( const T& value );
    void remove( K key );

    bool contains( K key ) const;
    bool isEmpty() const;
    size_t getCount() const;
private:
    Node& getNode( K key );
    void merge( K key1, K key2 );
    void split( K key );
    void borrowFromRight( K key );
    void borrowFromLeft( K key );
private:
    size_t _minDegree;
    SharedPtr<Node> _root;
};

#include "BTree.tpp"

#endif // BTREE_H