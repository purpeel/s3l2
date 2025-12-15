template <typename T>
UniquePtr<T>& UniquePtr<T>::operator=( UniquePtr<T>&& other ) {
    if (this != &other ) {
        delete this->_ptr;
        this->_ptr = other.release();
    }
    return *this;
}

template <typename T>
template <typename T2> requires (std::is_base_of_v<T,T2>)
UniquePtr<T>& UniquePtr<T>::operator=( UniquePtr<T2>&& other ) {
    if (static_cast<void*>(this) != static_cast<void*>(&other)) {
        delete this->_ptr;
        this->_ptr = other.release();
    }
    return *this;
}

template <typename T>
T& UniquePtr<T>::operator*() {
    if (this->_ptr == nullptr) {
        throw Exception( Exception::ErrorCode::NULL_DEREFERENCE );
    }
    return *this->_ptr;
}

template <typename T>
const T& UniquePtr<T>::operator*() const {
    if (this->_ptr == nullptr) {
        throw Exception( Exception::ErrorCode::NULL_DEREFERENCE );
    }
    return *this->_ptr;
}

template <typename T>
UniquePtr<T>::operator bool() const noexcept {
    return this->_ptr != nullptr;
}

template <typename T>
void UniquePtr<T>::reset() noexcept {
    delete this->_ptr;
    this->_ptr = nullptr;
}

template <typename T>
T* UniquePtr<T>::release() noexcept {
    auto *res = this->_ptr;
    this->_ptr = nullptr;
    return res;
}

template <typename T>
void UniquePtr<T>::swap( UniquePtr<T>& other ) noexcept {
    auto *tmp = this->_ptr;
    this->_ptr = other._ptr;
    other._ptr = tmp;
}

template <typename T>
bool UniquePtr<T>::operator==( T* const& other ) const noexcept {
    return this->_ptr == other;
}

template <typename T>
bool UniquePtr<T>::operator!=( T* const& other ) const noexcept {
    return this->_ptr != other;
}