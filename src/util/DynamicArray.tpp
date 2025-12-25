template <typename T>
DynamicArray<T>::DynamicArray() {
    try {
        this->size = 0;
        this->capacity = 3;
        this->allocBegin = new T[this->capacity];
        this->data = this->allocBegin + 1;
        this->allocEnd = this->allocBegin + this->capacity;
    } catch ( std::bad_alloc &ex ) {
        throw Exception(ex);
    }
}

template <typename T>
DynamicArray<T>::DynamicArray( const size_t capacity ) {
    try {
        this->size = 0;
        this->capacity = capacity;
        this->allocBegin = new T[this->capacity];
        this->data = this->allocBegin + (this->capacity / 4 + 1);
        this->allocEnd = this->allocBegin + this->capacity;
    } catch ( std::bad_alloc &ex ) {
        throw Exception(ex);
    }
}

template <typename T>
DynamicArray<T>::DynamicArray( const DynamicArray<T>& src ) {
    this->size = src.size;
    this->capacity = src.capacity;
    this->allocBegin = new T[this->capacity];
    this->data = this->allocBegin + (src.data - src.allocBegin);
    this->allocEnd = this->allocBegin + this->capacity;

    for ( size_t index = 0; index < this->size; index++ ) {
        (*this)[index] = src[index];
    }
}

template <typename T>
DynamicArray<T>& DynamicArray<T>::operator=( const DynamicArray<T>& src ) {
    if ( this != &src ) {
        delete[] this->allocBegin;

        this->size = src.size;
        this->capacity = src.capacity;
        this->allocBegin = new T[this->capacity];
        this->data = this->allocBegin + (src.data - src.allocBegin);
        this->allocEnd = this->allocBegin + this->capacity;
        for ( size_t index = 0; index < this->size; index++ ) {
            (*this)[index] = src[index];
        }
    }
    return *this;
}

template <typename T>
DynamicArray<T>::DynamicArray( DynamicArray<T>&& src ) {
    this->size = src.size;
    this->capacity = src.capacity;
    this->allocBegin = src.allocBegin;
    this->data = src.data;
    this->allocEnd = src.allocEnd;

    src.size = 0;
    src.capacity = 2;
    src.allocBegin = new T[src.capacity];
    src.data = src.allocBegin + 1;
    src.allocEnd = src.allocBegin + src.capacity;
}

template <typename T>
DynamicArray<T>& DynamicArray<T>::operator=( DynamicArray<T>&& src ) {
    if ( this != &src ) {
        delete[] this->allocBegin;
        this->size = src.size;
        this->capacity = src.capacity;
        this->allocBegin = src.allocBegin;
        this->data = src.data;
        this->allocEnd = src.allocEnd;

        src.size = 0;
        src.capacity = 2;
        src.allocBegin = new T[src.capacity];
        src.data = src.allocBegin + 1;
        src.allocEnd = src.allocBegin + src.capacity;
    }

    return *this;
}

template <typename T>
DynamicArray<T>::~DynamicArray() {
    delete[] this->allocBegin;
}

template <typename T>
void DynamicArray<T>::append( const T& value ) {
    int sizeDiff = 1;
    try {
        this->extend( sizeDiff );
        (*this)[this->size - 1] = value;
    } catch( std::bad_alloc &ex ) {
        throw Exception(ex);
    }
}

template <typename T>
void DynamicArray<T>::prepend( const T& value ) {
    int sizeDiff = 1;
    try {
        this->extend( sizeDiff );
        this->data--;
        (*this)[0] = value;
    } catch( std::bad_alloc &ex ) {
        throw Exception(ex);
    }
}

template <typename T>
void DynamicArray<T>::insertAt( const T& value, const size_t pos ) {
    if ( pos > this->size ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    if (pos == 0) {
        prepend(value);
    } else if (pos == this->getSize()) {
        append(value);
    } else {
        int sizeDiff = 1;
        this->extend( sizeDiff );
        for ( size_t index = this->size; index >= pos; index-- ) {
            *(data + index) = *(data + index - 1);
        }
        (*this)[pos] = value;
    }
}

template <typename T>
void DynamicArray<T>::setAt( const T& value, const size_t pos ) {
    if ( pos > this->size ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    this->data[pos] = value;
}

template <typename T>
void DynamicArray<T>::removeAt( const size_t pos ) {
    if ( pos > this->size ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    int sizeDiff = 1;
    try {
        T temp;
        for ( size_t index = pos; index < this->size - 1; index++ ) {
            temp = (*this)[index];
            (*this)[index] = (*this)[index + 1];
            (*this)[index + 1] = temp;
        }
        shrink( sizeDiff );
    } catch ( std::bad_alloc &ex ) {
        throw Exception(ex);
    }
}

template <typename T>
void DynamicArray<T>::swap( const size_t pos1, const size_t pos2 ) {
    if ( pos1 >= this->size || pos2 >= this->size ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    T temp = this->data[pos1];
    this->data[pos1] = this->data[pos2];
    this->data[pos2] = temp;
}

template <typename T>
void DynamicArray<T>::extend( int sizeDiff ) {
    if ( sizeDiff < 0 ) {
        throw Exception( Exception::ErrorCode::NEGATIVE_SIZE_DIFFERENCE );
    }

    this->size += sizeDiff;
    auto newCapacity = this->capacity;

    if ( newCapacity < 10000 ) {
        newCapacity = ( this->size > newCapacity * 0.75 ) 
            ? newCapacity * 2
            : newCapacity;
    } else {
        newCapacity = ( this->size > newCapacity * 0.25 )
            ? newCapacity + 1000
            : newCapacity;
    }
    if ( newCapacity != this->capacity ) {
        T* newAllocBegin = new T[newCapacity];
        T* newData = newAllocBegin + (newCapacity / 4 + 1);
        T* newAllocEnd = newAllocBegin + newCapacity;
        for ( size_t index = 0; index < this->size - sizeDiff; index++ ) {
            newData[index] = this->data[index];
        }
        delete[] this->allocBegin;
        this->capacity = newCapacity;
        this->allocBegin = newAllocBegin;
        this->data = newData;
        this->allocEnd = newAllocEnd;    
    }
}

template <typename T>
void DynamicArray<T>::shrink( const int sizeDiff ) {
    if ( sizeDiff < 0 ) {
        throw Exception( Exception::ErrorCode::NEGATIVE_SIZE_DIFFERENCE );
    }

    this->size -= sizeDiff;
    auto newCapacity = this->capacity;

    if ( newCapacity < 10000 ) {
        newCapacity = ( this->size < newCapacity * 0.25 )
            ? newCapacity / 2
            : newCapacity;
    } else {
        newCapacity = ( this->size < newCapacity * 0.75 )
            ? newCapacity - 1000
            : newCapacity;
    }
    if ( newCapacity != this->capacity ) {
        T* newAllocBegin = new T[newCapacity];
        T* newData = newAllocBegin + (newCapacity / 4 + 1);
        T* newAllocEnd = newAllocBegin + newCapacity;
        for ( size_t index = 0; index < this->size; index++ ) {
            newData[index] = this->data[index];
        }
        delete[] this->allocBegin; 
        this->capacity = newCapacity;
        this->allocBegin = newAllocBegin;
        this->data = newData;
        this->allocEnd = newAllocEnd;    
    }
}

template <typename T>
DynamicArray<T> DynamicArray<T>::subArray( const size_t startIndex, const size_t endIndex ) const {
    if ( startIndex > endIndex || startIndex >= this->getSize() || endIndex > this->getSize() ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    DynamicArray<T> res = DynamicArray<T>(endIndex - startIndex);
    for ( size_t index = 0; index < endIndex - startIndex; index++ ) {
        res.append((*this)[index + startIndex]);
    }
    return res;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::concat( const DynamicArray<T>& other ) {
    DynamicArray<T>* res = new DynamicArray<T>(this->size + other.size);
    for ( size_t index = 0; index < this->size; index++ ) {
        (*res)[index] = (*this)[index];
    }
    for ( int index = 0; index < other.size; index++ ) {
        (*res)[this->size + index] = other[index];
    }
    return res;
}

template <typename T>
T& DynamicArray<T>::operator[]( const size_t index ) {
    if ( index >= this->size ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    return this->data[index];
}

template <typename T>
const T& DynamicArray<T>::operator[]( const size_t index ) const {
    if ( index >= this->size ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    return this->data[index];
}

template <typename T>
void DynamicArray<T>::clear() {
    this->size = 0;
    this->capacity = 2;
    delete[] this->allocBegin;
    this->allocBegin = new T[this->capacity];
    this->data = this->allocBegin + (this->capacity / 4 + 1);
    this->allocEnd = this->allocBegin + this->capacity;
}

template <typename T>
size_t DynamicArray<T>::getSize() const {
    return this->size;
}

template <typename T>
size_t DynamicArray<T>::getCapacity() const {
    return this->capacity;
}

template <typename T>
template <typename T2>
void DynamicArray<T>::map( const std::function<T2(T&)>& func ) {
    for ( size_t index = 0; index < this->size; index++ ) {
        (*this)[index] = func((*this)[index]);
    }
}

template <typename T>
void DynamicArray<T>::where( const std::function<bool(T)>& func ) {
    for ( size_t index = 0; index < this->size; index++ ) {
        if (!func((*this)[index])) {
            removeAt(index);
        }
    }
}

template <typename T>
const std::string DynamicArray<T>::print() const {
    std::string result = "{";
    for ( size_t index = 0; index < this->size; index++ ) {
        // result += std::to_string( this->data[index] ) + " ";
    }
    result += "}";
    return result;
}

template <typename T>
bool DynamicArray<T>::isEmpty() const {
    return this->size == 0;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::appendImmutable( const T& value ) const {
    try {
        DynamicArray<T>* res = new DynamicArray<T>(*this);
        res->append( value );
        return res;
    } catch ( Exception& ex ) {
        throw Exception(ex);
    }
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::prependImmutable( const T& value ) const {
    try {
        DynamicArray<T>* res = new DynamicArray<T>(*this);
        res->prepend( value );
        return res;
    } catch ( Exception& ex ) {
        throw Exception(ex);
    }
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::insertAtImmutable( const T& value, const size_t pos ) const {
    try {
        DynamicArray<T>* res = new DynamicArray<T>(*this);
        res->insertAt( value, pos );
        return res;
    } catch ( Exception& ex ) {
        throw Exception(ex);
    }
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::removeAtImmutable( const size_t pos ) const {
    try {
        DynamicArray<T>* res = new DynamicArray<T>(*this);
        res->removeAt(pos);
        return res;
    } catch ( Exception& ex ) {
        throw Exception(ex);
    }
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::setAtImmutable( const T& value, const size_t pos ) const {
    try {
        DynamicArray<T>* res = new DynamicArray<T>(*this);
        res->setAt( value, pos );
        return res;
    } catch ( Exception& ex ) {
        throw Exception(ex);
    }
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::swapImmutable(const size_t pos1, const size_t pos2) const {
    try {
        DynamicArray<T>* res = new DynamicArray<T>(*this);
        res->swap(pos1, pos2);
        return res;
    } catch ( Exception& ex ) {
        throw Exception(ex);
    }
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::concatImmutable( const DynamicArray<T>& other ) const {
    try {
        DynamicArray<T>* res = new DynamicArray<T>(*this);
        return res->concat(other);
    } catch ( Exception& ex ) {
        throw Exception(ex);
    }
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::mapImmutable( const std::function<T(T)>& func ) const {
    auto res = new DynamicArray<T>(*this);
    res->map(func);
    return res;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::whereImmutable( const std::function<bool(T)>& func ) const {
    auto res = new DynamicArray<T>(*this);
    res->where(func);
    return res;
}

