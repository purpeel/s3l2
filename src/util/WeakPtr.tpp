template <typename T>
WeakPtr<T>::WeakPtr( const WeakPtr<T>& other ) : _ptr(other._ptr) { 
    _controlBlock = other._controlBlock;
    _controlBlock->increaseWeakRefs(); 
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=( const WeakPtr<T>& other ) {
    if (_controlBlock) {
        _controlBlock->decreaseWeakRefs();
        if (!_controlBlock->hasWeakRefs() && !_controlBlock->hasHardRefs()) { delete _controlBlock; }
    }
    _ptr = other._ptr;
    _controlBlock = other._controlBlock;
    _controlBlock->increaseWeakRefs();
    return *this;
}

template <typename T>
WeakPtr<T>::WeakPtr( const SharedPtr<T>& other ) : _ptr(other._ptr) {
    _controlBlock = other._controlBlock;
    _controlBlock->increaseWeakRefs();
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=( const SharedPtr<T>& other ) {
    if (_controlBlock) {
        _controlBlock->decreaseWeakRefs();
        if (!_controlBlock->hasWeakRefs() && !_controlBlock->hasHardRefs()) { delete _controlBlock; }
    }
    _ptr = other._ptr;
    _controlBlock = other._controlBlock;
    _controlBlock->increaseWeakRefs();
    return *this;
}

template <typename T>
template<typename T2> requires (std::is_base_of_v<T,T2>)
WeakPtr<T>::WeakPtr( const WeakPtr<T2>& other ) : _ptr(other._ptr) { 
    _controlBlock = other._controlBlock;
    _controlBlock->increaseWeakRefs(); 
}

template <typename T>
template<typename T2> requires (std::is_base_of_v<T,T2>)
WeakPtr<T>& WeakPtr<T>::operator=( const WeakPtr<T2>& other ) {
    if (_controlBlock) {
        _controlBlock->decreaseWeakRefs();
        if (!_controlBlock->hasWeakRefs() && !_controlBlock->hasHardRefs()) { delete _controlBlock; }
    }
    _ptr = other._ptr;
    _controlBlock = other._controlBlock;
    _controlBlock->increaseWeakRefs();
    return *this;
}

template <typename T>
template<typename T2> requires (std::is_base_of_v<T,T2>)
WeakPtr<T>::WeakPtr( const SharedPtr<T2>& other ) : _ptr(other._ptr) {
    _controlBlock = other._controlBlock;
    _controlBlock->increaseWeakRefs();
}

template <typename T>
template<typename T2> requires (std::is_base_of_v<T,T2>)
WeakPtr<T>& WeakPtr<T>::operator=( const SharedPtr<T2>& other ) {
    if (_controlBlock) {
        _controlBlock->decreaseWeakRefs();
        if (!_controlBlock->hasWeakRefs() && !_controlBlock->hasHardRefs()) { delete _controlBlock; }
    }
    _ptr = other._ptr;
    _controlBlock = other._controlBlock;
    _controlBlock->increaseWeakRefs();
    return *this;
}

template <typename T>
WeakPtr<T>::~WeakPtr() {
    if (_controlBlock) {
        _controlBlock->decreaseWeakRefs();
        if ( !_controlBlock->hasWeakRefs() && !_controlBlock->hasHardRefs() ) {
            delete _controlBlock;
        }
    }
}

template <typename T>
void WeakPtr<T>::reset() noexcept {
    _ptr = nullptr;
    _controlBlock->decreaseWeakRefs();
    if (_controlBlock->weakRefs() == 0 && _controlBlock->hardRefs() == 0) {
        delete _controlBlock;
    }
    _controlBlock = nullptr;
}

template <typename T>
SharedPtr<T> WeakPtr<T>::lock() const noexcept {
    return SharedPtr<T>(_ptr, _controlBlock);
}

template <typename T>
void WeakPtr<T>::swap( WeakPtr<T>& other ) noexcept {
    auto *temp = _ptr;
    _ptr = other._ptr;
    other._ptr = temp;

    auto *tempCount = _controlBlock;
    _controlBlock = other._controlBlock;
    other._controlBlock = temp;
}

template <typename T>
WeakPtr<T>::operator bool() const noexcept {
    return _ptr;
}

template <typename T>
bool WeakPtr<T>::operator==( const WeakPtr<T>& other ) const noexcept {
    return _ptr == other._ptr;
}

template <typename T>
bool WeakPtr<T>::operator==( const SharedPtr<T>& other ) const noexcept {
    return _ptr == other._ptr;
}

template <typename T>
bool WeakPtr<T>::operator==( T* const& other ) const noexcept {
    return _ptr == other;
}

template <typename T>
bool WeakPtr<T>::operator!=( const WeakPtr<T>& other ) const noexcept {
    return _ptr != other._ptr;
}

template <typename T>
bool WeakPtr<T>::operator!=( const SharedPtr<T>& other ) const noexcept {
    return _ptr != other._ptr;
}

template <typename T>
bool WeakPtr<T>::operator!=( T* const& other ) const noexcept {
    return _ptr != other;
}

template <typename T>
template<typename T2> requires (std::is_base_of_v<T,T2>)
bool WeakPtr<T>::operator==( const WeakPtr<T2>& other ) const noexcept {
    return _ptr == other._ptr;
}

template <typename T>
template<typename T2> requires (std::is_base_of_v<T,T2>)
bool WeakPtr<T>::operator==( const SharedPtr<T2>& other ) const noexcept {
    return _ptr == other._ptr;
}

template <typename T>
template<typename T2> requires (std::is_base_of_v<T,T2>)
bool WeakPtr<T>::operator==( T2* const& other ) const noexcept {
    return _ptr == other;
}

template <typename T>
template<typename T2> requires (std::is_base_of_v<T,T2>)
bool WeakPtr<T>::operator!=( const WeakPtr<T2>& other ) const noexcept {
    return _ptr != other._ptr;
}

template <typename T>
template<typename T2> requires (std::is_base_of_v<T,T2>)
bool WeakPtr<T>::operator!=( const SharedPtr<T2>& other ) const noexcept {
    return _ptr != other._ptr;
}

template <typename T>
template<typename T2> requires (std::is_base_of_v<T,T2>)
bool WeakPtr<T>::operator!=( T2* const& other ) const noexcept {
    return _ptr != other;
}