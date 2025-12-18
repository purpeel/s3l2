#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "SharedPtr.hpp"

template <typename T>
class Sequence 
{
public:
    virtual Sequence<T>* clone() const = 0;
    
    virtual void copy( const Sequence<T>& src ) = 0;

    virtual void clear() = 0;
    virtual ~Sequence() = default;
public:
    virtual void append( const T& value ) = 0;
    virtual void prepend( const T& value ) = 0;
    virtual void insertAt( const T& value, const int pos ) = 0;
    virtual void removeAt( const int pos ) = 0;
    virtual void setAt( const T& value, const int pos ) = 0;
    virtual void swap( const int pos1, const int pos2 ) = 0;
    virtual Sequence<T>* getSubSequence( const int startIndex, const int endIndex ) const = 0;
    virtual Sequence<T>* concat( const Sequence<T>& other ) = 0;
public:
    virtual T& operator[]( const int pos ) = 0;
    virtual const T& operator[]( const int pos ) const = 0;
public:
    virtual bool isEmpty() const = 0;
    virtual size_t getSize() const = 0;
public: 
    virtual const std::string print() const = 0;
public: // immutable functions
    virtual Sequence<T>* appendImmutable( const T& value ) const = 0;
    virtual Sequence<T>* prependImmutable( const T& value ) const = 0;
    virtual Sequence<T>* insertAtImmutable( const T& value, const int pos ) const = 0;
    virtual Sequence<T>* removeAtImmutable( const int pos ) const = 0;
    virtual Sequence<T>* setAtImmutable( const T& value, const int pos ) const = 0;
    virtual Sequence<T>* swapImmutable( const int pos1, const int pos2 ) const = 0;
    virtual Sequence<T>* concatImmutable( const Sequence<T>& other ) const = 0;
};

#endif // SEQUENCE_H