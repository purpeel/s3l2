#ifndef SHARED_PTR
#define SHARED_PTR

#include "util.hpp"

template <typename T>
class UniquePtr;
template <typename T>
class WeakPtr;
template <typename T>
class EnableSharedFromThis;

struct RefCount
{
public:
    RefCount() : _hardRefs( 0 ), _weakRefs( 0 ) {}
    RefCount( const long& hardRefs, const long& weakRefs ) 
    : _hardRefs( hardRefs ), _weakRefs( weakRefs ) {}
    ~RefCount() {}
public:
    long hardRefs() { return _hardRefs; }
    long weakRefs() { return _weakRefs; }

    bool hasHardRefs() { return _hardRefs != 0; }
    bool hasWeakRefs() { return _weakRefs != 0; }

    void increaseHardRefs() { _hardRefs++; }
    void increaseWeakRefs() { _weakRefs++; }
    
    void decreaseHardRefs() { _hardRefs--; }
    void decreaseWeakRefs() { _weakRefs--; }
private: 
    long _hardRefs;
    long _weakRefs;
};

template <class T>
class SharedPtr
{
private:
    SharedPtr( T* ptr ) : _ptr( ptr ), _controlBlock( new RefCount(1, 0) ) { hookSharedToThis(ptr); }
    SharedPtr( T* ptr, RefCount* count ) : _ptr( ptr ), _controlBlock( count ) {
        _controlBlock->increaseHardRefs();
        hookSharedToThis(ptr);
    }

    template<typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr( T2* ptr ) : _ptr( ptr ), _controlBlock( new RefCount(1, 0) ) { hookSharedToThis(_ptr); }
    template<typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr( T2* ptr, RefCount* count ) : _ptr( ptr ), _controlBlock( count ) {
        _controlBlock->increaseHardRefs();
        hookSharedToThis(ptr);
    }
    
    friend class EnableSharedFromThis<T>;
    friend class WeakPtr<T>;
    template <typename T2> 
    friend class SharedPtr;
    template <typename T2>
    friend class WeakPtr;
    template <typename T2, typename... Ts>
    friend SharedPtr<T2> makeShared(Ts&&... args) requires (!std::is_abstract_v<T2>);
    template <typename T2, typename... Ts>
    friend SharedPtr<T2> makeShared(Ts&&... args) requires (std::is_abstract_v<T2>);

    template <typename T2> requires (std::is_base_of_v<T,T2>)
    void hookSharedToThis( T2* ptr );
    static void manageControlChange( T *& ptr, RefCount *& controlBlock );
public:
    SharedPtr() requires(!std::is_abstract_v<T>) : _ptr(new T()) , _controlBlock( new RefCount(1, 0) ) { hookSharedToThis(_ptr); } 
    SharedPtr() requires(std::is_abstract_v<T>) : _ptr(nullptr) , _controlBlock(nullptr) {} 

    SharedPtr( UniquePtr<T>&& other, RefCount* count ) : _ptr(other.release()), _controlBlock( count ) { hookSharedToThis(_ptr); }
    SharedPtr( UniquePtr<T>&& other ) : _ptr(other.release()), _controlBlock( new RefCount(1, 0)) { hookSharedToThis(_ptr); }
    SharedPtr<T>& operator=( UniquePtr<T>&& other );

    SharedPtr( const SharedPtr<T>& other );
    SharedPtr<T>& operator=( const SharedPtr<T>& other );
    SharedPtr( SharedPtr<T>&& other );
    SharedPtr<T>& operator=( SharedPtr<T>&& other );

    SharedPtr( const WeakPtr<T>& other );
    SharedPtr<T>& operator=( const WeakPtr<T>& other );
    SharedPtr( WeakPtr<T>&& other );
    SharedPtr<T>& operator=( WeakPtr<T>&& other );

    ~SharedPtr();
public:
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr( UniquePtr<T2>&& other, RefCount* count ) : _ptr(other.release()), _controlBlock( count ) { hookSharedToThis(_ptr); }

    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr( UniquePtr<T2>&& other ) : _ptr(other.release()), _controlBlock( new RefCount(1, 0)) { hookSharedToThis(_ptr); }
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr<T>& operator=( UniquePtr<T2>&& other );

    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr( const SharedPtr<T2>& other ) : _ptr( other._ptr ), _controlBlock(other._controlBlock) { 
        if (_controlBlock) { _controlBlock->increaseHardRefs(); }
    }
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr<T>& operator=( const SharedPtr<T2>& other );

    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr( SharedPtr<T2>&& other );
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr<T>& operator=( SharedPtr<T2>&& other );

    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr( const WeakPtr<T2>& other );
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr<T>& operator=( const WeakPtr<T2>& other );

    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr( WeakPtr<T2>&& other );
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    SharedPtr<T>& operator=( WeakPtr<T2>&& other );
public:
    operator T*() noexcept {
        return _ptr;
    }
    T* operator->() noexcept {
        return _ptr;
    }
    const T* operator->() const noexcept {
        return _ptr;
    }

    T& operator*();
    const T& operator*() const;

    operator bool() const noexcept;
    bool isUnique() {
        return _controlBlock->hardRefs() == 1;
    }
public:
    void reset() noexcept;

    void swap( SharedPtr<T>& other ) noexcept;
    bool isUnique() const noexcept {
        return _controlBlock->hardRefs() == 1;
    }

    int getCount() const noexcept {
        return _controlBlock->hardRefs();
    }
public:
    bool operator==( const SharedPtr<T>& other ) const noexcept;
    bool operator==( T* const& other ) const noexcept;
    bool operator!=( const SharedPtr<T>& other ) const noexcept;
    bool operator!=( T* const& other ) const noexcept;

    template <typename T2> requires (std::is_base_of_v<T,T2>)
    bool operator==( const SharedPtr<T2>& other ) const noexcept;
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    bool operator==( T2* const& other ) const noexcept;
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    bool operator!=( const SharedPtr<T2>& other ) const noexcept;
    template <typename T2> requires (std::is_base_of_v<T,T2>)
    bool operator!=( T2* const& other ) const noexcept;
private:
    T* _ptr;

    RefCount* _controlBlock;

    using weakType = WeakPtr<T>;
};

template <typename T, typename ... Ts>
SharedPtr<T> makeShared(Ts&& ... args ) requires(!std::is_abstract_v<T>) {
    return SharedPtr<T>( new T( std::forward<Ts>(args)...));
}

template <typename T, typename ... Ts>
SharedPtr<T> makeShared(Ts&& ... args ) requires(std::is_abstract_v<T>) = delete;

#include "SharedPtr.tpp"

#endif // SHARED_PTR
