#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "util.hpp"
#include <functional>

template <typename T> 
class DynamicArray 
{
public:
    DynamicArray();
    DynamicArray( const size_t capacity );

    DynamicArray( const DynamicArray<T>& other );
    DynamicArray<T>& operator=( const DynamicArray<T>& other );
    
    DynamicArray( DynamicArray<T>&& other);
    DynamicArray<T>& operator=( DynamicArray<T>&& other );
    
    virtual ~DynamicArray();
    virtual void clear();
public:
    void append( const T& value );
    void prepend( const T& value );
    void setAt( const T& value, const size_t pos );
    void insertAt( const T& value, size_t pos );
    void removeAt( const size_t pos );
    void swap( const size_t pos1, const size_t pos2 );
    DynamicArray<T> subArray( const size_t startIndex, const size_t endIndex ) const;
    DynamicArray<T>* concat( const DynamicArray<T>& other );
private:
    void extend( const int sizeDiff );
    void shrink( const int sizeDiff );
public:
    T& operator[]( const size_t pos );
    const T& operator[]( const size_t pos ) const;
public:
    size_t getSize() const;
    bool isEmpty() const;
public:
    DynamicArray<T>* appendImmutable( const T& value ) const;
    DynamicArray<T>* prependImmutable( const T& value ) const;
    DynamicArray<T>* insertAtImmutable( const T& value, const size_t pos ) const;
    DynamicArray<T>* removeAtImmutable( const size_t pos ) const;
    DynamicArray<T>* setAtImmutable( const T& value, const size_t pos ) const;
    DynamicArray<T>* swapImmutable( const size_t pos1, const size_t pos2 ) const;
    DynamicArray<T>* concatImmutable( const DynamicArray<T>& other ) const;
    DynamicArray<T>* mapImmutable( const std::function<T(T)>& func ) const;
    DynamicArray<T>* whereImmutable( const std::function<bool(T)>& func ) const;
public:
    void map( const std::function<T(T&)>& func );
    void where( const std::function<bool(T)>& func );
private:
    size_t getCapacity() const;
    T* _allocBegin;
    T* _data;
    T* _allocEnd;
    size_t _size;
    size_t _capacity;
    size_t _offset;
};

#include "DynamicArray.tpp"
#endif // DYNAMIC_ARRAY_H