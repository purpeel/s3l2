#ifndef WEAK_PTR_H
#define WEAK_PTR_H

#include "util.hpp"
#include "SharedPtr.hpp"

struct RefCount;

template <class T>
class WeakPtr 
{
public: 
    WeakPtr() : _ptr( nullptr ), _controlBlock( new RefCount(0, 1) ) {}

    WeakPtr( const WeakPtr<T>& other );
    WeakPtr& operator=( const WeakPtr<T>& other );

    WeakPtr( const SharedPtr<T>& other );
    WeakPtr<T>& operator=( const SharedPtr<T>& other );

    ~WeakPtr();
public:
    template<typename T2> requires (std::is_base_of_v<T,T2>)
    WeakPtr( const WeakPtr<T2>& other );
    template<typename T2> requires (std::is_base_of_v<T,T2>)
    WeakPtr& operator=( const WeakPtr<T2>& other );

    template<typename T2> requires (std::is_base_of_v<T,T2>)
    WeakPtr( const SharedPtr<T2>& other );
    template<typename T2> requires (std::is_base_of_v<T,T2>)
    WeakPtr<T>& operator=( const SharedPtr<T2>& other );
public:
    void reset() noexcept;

    long getSharedCount() const noexcept {
        return this->_controlBlock->hardRefs();
    }
    long getWeakCount() const noexcept {
        return this->_controlBlock->weakRefs();
    }
    SharedPtr<T> lock() const noexcept;

    void swap( WeakPtr<T>& other ) noexcept;

    operator bool() const noexcept;

    bool isExpired() { 
        return (_controlBlock) ? !_controlBlock->hasHardRefs() : true;
    }
public:
    bool operator==( const WeakPtr<T>& other ) const noexcept;
    bool operator==( const SharedPtr<T>& other ) const noexcept;
    bool operator==( T* const& other ) const noexcept;
    bool operator!=( const WeakPtr<T>& other ) const noexcept;
    bool operator!=( const SharedPtr<T>& other ) const noexcept;
    bool operator!=( T* const& other ) const noexcept;

    template<typename T2> requires (std::is_base_of_v<T,T2>)
    bool operator==( const WeakPtr<T2>& other ) const noexcept;
    template<typename T2> requires (std::is_base_of_v<T,T2>)
    bool operator==( const SharedPtr<T2>& other ) const noexcept;
    template<typename T2> requires (std::is_base_of_v<T,T2>)
    bool operator==( T2* const& other ) const noexcept;
    template<typename T2> requires (std::is_base_of_v<T,T2>)
    bool operator!=( const WeakPtr<T2>& other ) const noexcept;
    template<typename T2> requires (std::is_base_of_v<T,T2>)
    bool operator!=( const SharedPtr<T2>& other ) const noexcept;
    template<typename T2> requires (std::is_base_of_v<T,T2>)
    bool operator!=( T2* const& other ) const noexcept;
private:
    T* _ptr;
    
    RefCount* _controlBlock;

    friend class SharedPtr<T>;
    template <typename T2>
    friend class SharedPtr;
    template <typename T2>
    friend class WeakPtr;
};

#include "WeakPtr.tpp"

#endif //WEAK_PTR_H
