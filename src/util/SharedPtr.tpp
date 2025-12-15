template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
void SharedPtr<T>::hookSharedToThis( T2* ptr ) {
    if (ptr) {
        if constexpr( std::is_base_of_v<EnableSharedFromThis<T>,T2> ) {
            auto *base = static_cast<EnableSharedFromThis<T>*>( ptr );
            if (base->_self.isExpired()) {
                base->_self = *this;
            }
        }
    }
}

template <typename T>
void SharedPtr<T>::manageControlChange( T *& ptr, RefCount *& controlBlock ) {
    if (controlBlock) {
        controlBlock->decreaseHardRefs();
        bool noHard = !controlBlock->hasHardRefs();
        bool noWeak = !controlBlock->hasWeakRefs();
        if (noHard) { 
            delete ptr; 
            ptr = nullptr;
            if (noWeak) { 
                delete controlBlock;
                controlBlock = nullptr;
            }
        }
    }
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=( UniquePtr<T>&& other ) {
    manageControlChange( _ptr, _controlBlock );
    _ptr = other.release();
    _controlBlock = new RefCount(1, 0);
    hookSharedToThis(_ptr);
    return *this;    
}

template <typename T>
SharedPtr<T>::SharedPtr( const SharedPtr<T>& other ) : _ptr( other._ptr ) { 
    _controlBlock = other._controlBlock;
    if (_controlBlock) {
        _controlBlock->increaseHardRefs();
    } else {
        _controlBlock = new RefCount(1, 0);
    }
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=( const SharedPtr<T>& other ) {
    if (this != &other) {
        manageControlChange( _ptr, _controlBlock );
        _ptr = other._ptr;
        _controlBlock = other._controlBlock;
        _controlBlock->increaseHardRefs();
    }
    return *this;
}

template <typename T>
SharedPtr<T>::SharedPtr( SharedPtr<T>&& other ) {
    _ptr = other._ptr;
    _controlBlock = other._controlBlock;
    other._ptr = nullptr;
    other._controlBlock = nullptr;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=( SharedPtr<T>&& other ) {
    if (this != &other) {
        manageControlChange( _ptr, _controlBlock );
        _ptr = other._ptr;
        _controlBlock = other._controlBlock;
        other._ptr = nullptr;
        other._controlBlock = nullptr;
    }
    return *this;
}


template <typename T>
SharedPtr<T>::SharedPtr( const WeakPtr<T>& other ) : _ptr( other._ptr ) { 
    _controlBlock = other._controlBlock;
    _controlBlock->increaseHardRefs();
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=( const WeakPtr<T>& other ) {
    if (this != &other) {
        manageControlChange( _ptr, _controlBlock );
        _ptr = other._ptr;
        _controlBlock = other._controlBlock;
        _controlBlock->increaseHardRefs();
    }
    return *this;
}

template <typename T>
SharedPtr<T>::SharedPtr( WeakPtr<T>&& other ) {
    _ptr = other._ptr;
    _controlBlock = other._controlBlock;
    other._ptr = nullptr;
    other._controlBlock = nullptr;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=( WeakPtr<T>&& other ) {
    if (this != &other) {
        manageControlChange( _ptr, _controlBlock );
        _ptr = other._ptr;
        _controlBlock = other._controlBlock;
        other._ptr = nullptr;
        other._controlBlock = nullptr;
    }
    return *this;
}

template <typename T>
SharedPtr<T>::~SharedPtr() {
    manageControlChange( _ptr, _controlBlock );
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
SharedPtr<T>& SharedPtr<T>::operator=( UniquePtr<T2>&& other ) {
    _ptr = other.release;
    _controlBlock = new RefCount(1, 0);
    hookSharedToThis(_ptr);

    return *this;
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
SharedPtr<T>& SharedPtr<T>::operator=( const SharedPtr<T2>& other ) {
    if (this != &other) {
        manageControlChange( _ptr, _controlBlock );
        _ptr = other._ptr;
        _controlBlock = other._controlBlock;
        _controlBlock->increaseHardRefs();
    }
    return *this;
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
SharedPtr<T>::SharedPtr( SharedPtr<T2>&& other ) {
    _ptr = other._ptr;
    _controlBlock = other._controlBlock;
    other._ptr = nullptr;
    other._controlBlock = nullptr;
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
SharedPtr<T>& SharedPtr<T>::operator=( SharedPtr<T2>&& other ) {
    if (static_cast<void*>(this) != static_cast<void*>(&other)) {
        manageControlChange( _ptr, _controlBlock );
        _ptr = other._ptr;
        _controlBlock = other._controlBlock;
        other._ptr = nullptr;
        other._controlBlock = nullptr;
    }
    return *this;
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
SharedPtr<T>::SharedPtr( const WeakPtr<T2>& other ) : _ptr(other._ptr) {
    _controlBlock = other._controlBlock;
    _controlBlock->increaseHardRefs();
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
SharedPtr<T>& SharedPtr<T>::operator=( const WeakPtr<T2>& other ) {
    if (this != &other) {
        manageControlChange( _ptr, _controlBlock );
        _ptr = other._ptr;
        _controlBlock = other._controlBlock;
        _controlBlock->increaseHardRefs();
    }
    return *this;
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
SharedPtr<T>::SharedPtr( WeakPtr<T2>&& other ) {
    _ptr = other._ptr;
    _controlBlock = other._controlBlock;
    other._ptr = nullptr;
    other._controlBlock = nullptr;
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
SharedPtr<T>& SharedPtr<T>::operator=( WeakPtr<T2>&& other ) {
    if (static_cast<void*>(this) != static_cast<void*>(&other)) {
        manageControlChange( _ptr, _controlBlock );
        _ptr = other._ptr;
        _controlBlock = other._controlBlock;
        other._ptr = nullptr;
        other._controlBlock = nullptr;
    }
    return *this;
}

template <typename T>
T& SharedPtr<T>::operator*() {
    if (!_ptr) {
        throw Exception( Exception::ErrorCode::NULL_DEREFERENCE );
    } 
    return *_ptr;
}

template <typename T>
const T& SharedPtr<T>::operator*() const {
    if (!_ptr) {
        throw Exception( Exception::ErrorCode::NULL_DEREFERENCE );
    } 
    return *_ptr;
}

template <typename T>
SharedPtr<T>::operator bool() const noexcept {
    return _ptr != nullptr;
}

template <typename T>
void SharedPtr<T>::reset() noexcept {
    manageControlChange( _ptr, _controlBlock );
}

template <typename T>
void SharedPtr<T>::swap( SharedPtr<T>& other ) noexcept {
    auto temp1 = _ptr;
    _ptr = other._ptr;
    other._ptr = temp1;

    auto temp2 = _controlBlock;
    _controlBlock = other._controlBlock;
    other._controlBlock = temp2;
}

template <typename T>
bool SharedPtr<T>::operator==( const SharedPtr<T>& other ) const noexcept {
    return _ptr == other._ptr;
}

template <typename T>
bool SharedPtr<T>::operator==( T* const& other ) const noexcept {
    return _ptr == other;
}


template <typename T>
bool SharedPtr<T>::operator!=( const SharedPtr<T>& other ) const noexcept {
    return _ptr != other._ptr;
}

template <typename T>
bool SharedPtr<T>::operator!=( T* const& other ) const noexcept {
    return _ptr != other;
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
bool SharedPtr<T>::operator==( const SharedPtr<T2>& other ) const noexcept {
    return _ptr == other._ptr;
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
bool SharedPtr<T>::operator==( T2* const& other ) const noexcept {
    return _ptr == other;
}


template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
bool SharedPtr<T>::operator!=( const SharedPtr<T2>& other ) const noexcept {
    return _ptr != other._ptr;
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
bool SharedPtr<T>::operator!=( T2* const& other ) const noexcept {
    return _ptr != other;
}