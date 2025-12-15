#ifndef UNIQUE_PTR
#define UNIQUE_PTR

#include <concepts>
#include <type_traits>
#include "SharedPtr.hpp"

template <class T>
class UniquePtr
{
public: 
    UniquePtr() requires (!std::is_abstract_v<T>) : _ptr( new T() ) {}
    
    UniquePtr() requires (std::is_abstract_v<T>) = delete;

    UniquePtr( const UniquePtr<T>& other ) = delete;
    UniquePtr<T>& operator=( const UniquePtr<T>& other ) = delete;

    UniquePtr( UniquePtr<T>&& other ) : _ptr( other.release() ) {}
    UniquePtr<T>& operator=( UniquePtr<T>&& other );

    ~UniquePtr() {
        if (this->_ptr) { delete this->_ptr; }
    }
public:
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    UniquePtr( UniquePtr<T2>&& other ) : _ptr( other.release() ) {}
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    UniquePtr<T>& operator=( UniquePtr<T2>&& other );
private:
    UniquePtr( T* ptr ) : _ptr( ptr ) {}

    template<typename T2> requires (std::is_base_of_v<T,T2>)
    UniquePtr( T2* ptr ) : _ptr( ptr ) {}
    
    template <typename T2>
    friend class UniquePtr;
    template <typename T2, typename ... Ts>
    friend UniquePtr<T2> makeUnique( Ts&& ... args ) requires (!std::is_abstract_v<T2>);
    template <typename T2, typename ... Ts>
    friend UniquePtr<T2> makeUnique( Ts&& ... args ) requires (!std::is_abstract_v<T2>);
public:
    T* operator->() noexcept {
        return this->_ptr;
    }
    const T* operator->() const noexcept {
        return this->_ptr;
    }
    
    T& operator*();
    const T& operator*() const;

    operator bool() const noexcept;
public:
    void reset() noexcept;

    T* release() noexcept;

    void swap( UniquePtr<T>& other ) noexcept;
public:
    bool operator==( T* const& other ) const noexcept;
    bool operator!=( T* const& other ) const noexcept;
private:
    T* _ptr;
};

template <typename T, typename ... Ts>
static UniquePtr<T> makeUnique( Ts&& ... args ) requires (!std::is_abstract_v<T>) {
    return UniquePtr<T>( new T(std::forward<Ts>(args)...));
}

template <typename T, typename ... Ts>
static UniquePtr<T> makeUnique( Ts&& ... args ) requires (std::is_abstract_v<T>) = delete;

#include "UniquePtr.tpp"

#endif //UNIQUE_PTR