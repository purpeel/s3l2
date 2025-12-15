#ifndef SHARED_FROM_THIS_H
#define SHARED_FROM_THIS_H

#include "util.hpp"
#include "WeakPtr.hpp"

template <typename T>
class EnableSharedFromThis
{
public:
    SharedPtr<T> sharedFromThis() { return SharedPtr<T>( _self ); }
    const SharedPtr<T> sharedFromThis() const { return SharedPtr<T>( _self ); }

    WeakPtr<T> weakFromThis() { return _self; }
    const WeakPtr<T> weakFromThis() const { return _self; }
protected:
    EnableSharedFromThis() = default;
     
    EnableSharedFromThis( const EnableSharedFromThis<T>& other ) = default;
    EnableSharedFromThis<T>& operator=( const EnableSharedFromThis<T>& other ) = default;

    ~EnableSharedFromThis() = default;
private:
    WeakPtr<T> _self;
     
    friend class SharedPtr<T>;
};

#endif // SHARED_FROM_THIS_H