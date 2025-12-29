#ifndef ARRAYSEQ_H
#define ARRAYSEQ_H

#include "Sequence.hpp"
#include "DynamicArray.hpp"

template <typename T>
class ArraySequence : public Sequence<T>
{
public:
    ArraySequence();
    ArraySequence( const size_t capacity );

    ArraySequence( const DynamicArray<T>& src );
    ArraySequence( const ArraySequence<T>& src );
    ArraySequence<T>& operator=( const DynamicArray<T>& src );
    ArraySequence<T>& operator=( const ArraySequence<T>& src );

    ArraySequence( DynamicArray<T>&& src );
    ArraySequence( ArraySequence<T>&& src );
    ArraySequence<T>& operator=( DynamicArray<T>&& src );
    ArraySequence<T>& operator=( ArraySequence<T>&& src );

    Sequence<T>* clone() const override;

    void copy( const Sequence<T>& src ) override;
    void clear() override;
    virtual ~ArraySequence() = default;
public:
    void append( const T& value ) override;
    void prepend( const T& value ) override;
    void insertAt( const T& value, const size_t pos ) override;
    void removeAt( const size_t pos ) override;
    void setAt( const T& value, const size_t pos ) override;
    void swap( const size_t pos1, const size_t pos2 ) override;
    ArraySequence<T> subArray( const size_t startIndex, const size_t endIndex ) const;
    Sequence<T>* getSubSequence( const size_t startIndex, const size_t endIndex ) const override;
    Sequence<T>* concat( const Sequence<T>& other ) override;
    void map( const std::function<T(T&)>& func );
    void where( const std::function<bool(T)>& func );
public:
    T& operator[]( const size_t pos ) override;
    const T& operator[]( const size_t pos ) const override;
public:
    bool isEmpty() const override;
    size_t getSize() const override;
public:
    Sequence<T>* appendImmutable( const T& value ) const override;
    Sequence<T>* prependImmutable( const T& value ) const override;
    Sequence<T>* insertAtImmutable( const T& value, const size_t pos ) const override;
    Sequence<T>* removeAtImmutable( const size_t pos ) const override;
    Sequence<T>* setAtImmutable( const T& value, const size_t pos ) const override;
    Sequence<T>* swapImmutable( const size_t pos1, const size_t pos2 ) const override;
    Sequence<T>* concatImmutable( const Sequence<T>& other ) const override;
    Sequence<T>* mapImmutable( const std::function<T(T)>& func ) const;
    Sequence<T>* whereImmutable( const std::function<bool(T)>& func ) const;
private:
    DynamicArray<T> array;
};

#include "ArraySequence.tpp"
#endif // ARRAYSEQ_H