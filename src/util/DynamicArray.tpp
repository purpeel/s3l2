template <typename T>
DynamicArray<T>::DynamicArray() {
    _size = 0;
    _capacity = 2;
    _offset = 1;

    _allocBegin = new T[_capacity + _offset];
    _allocEnd = _allocBegin + (_capacity + _offset);
    _data = _allocBegin + _offset;
}

template <typename T>
DynamicArray<T>::DynamicArray( const size_t capacity ) {
    _size = 0;
    _capacity = capacity;
    _offset = capacity / 4 + 1;

    _allocBegin = new T[_capacity + _offset];
    _allocEnd = _allocBegin + (_capacity + _offset);
    _data = _allocBegin + _offset;
}

template <typename T>
DynamicArray<T>::DynamicArray( const DynamicArray<T>& other ) {
    _size = other._size;
    _capacity = other._capacity;
    _offset = other._offset;

    _allocBegin = new T[_capacity + _offset];
    _allocEnd = _allocBegin + (_capacity + _offset);
    _data = _allocBegin + _offset;

    for ( size_t index = 0; index < _size; index++ ) {
        _data[index] = other._data[index];
    } 
}

template <typename T>
DynamicArray<T>& DynamicArray<T>::operator=( const DynamicArray<T>& other ) {
    if ( this != &other ) {
        delete[] _allocBegin;

        _size = other._size;
        _capacity = other._capacity;
        _offset = other._offset;

        _allocBegin = new T[_capacity + _offset];
        _allocEnd = _allocBegin + (_capacity + _offset);
        _data = _allocBegin + _offset;

        for ( size_t index = 0; index < _size; index++ ) {
            _data[index] = other._data[index];
        } 
    }
    return *this;
}

template <typename T>
DynamicArray<T>::DynamicArray( DynamicArray<T>&& other ) {
    _size = other._size;
    _capacity = other._capacity;
    _offset = other._offset;
    _allocBegin = other._allocBegin;
    _data = other._data;
    _allocEnd = other._allocEnd;

    other._size = 0;
    other._capacity = 2;
    other._offset = 1;
    other._allocBegin = new T[other._capacity + other._offset];
    other._allocEnd = other._allocBegin + (other._capacity + other._offset);
    other._data = other._allocBegin + other._offset;
}

template <typename T>
DynamicArray<T>& DynamicArray<T>::operator=( DynamicArray<T>&& other ) {
    if ( this != &other ) {
        delete[] _allocBegin;
        
        _size = other._size;
        _capacity = other._capacity;
        _offset = other._offset;
        _allocBegin = other._allocBegin;
        _data = other._data;
        _allocEnd = other._allocEnd;

        other._size = 0;
        other._capacity = 2;
        other._offset = 1;
        other._allocBegin = new T[other._capacity + other._offset];
        other._allocEnd = other._allocBegin + (other._capacity + other._offset);
        other._data = other._allocBegin + other._offset;
    }

    return *this;
}

template <typename T>
DynamicArray<T>::~DynamicArray() {
    delete[] _allocBegin;
}

template <typename T>
void DynamicArray<T>::append( const T& value ) {
    extend(1);
    _data[_size - 1] = value;
}

template <typename T>
void DynamicArray<T>::prepend( const T& value ) {
    extend(1);
    _data--;
    _data[0] = value;
}

template <typename T>
void DynamicArray<T>::insertAt( const T& value, const size_t pos ) {
    if (pos > _size) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }

    if (pos == 0) { prepend(value); } 
    else if (pos == _size) { append(value); }
    else {
        extend(1);
        for (size_t index = _size - 1; index >= pos; index--) {
            _data[index + 1] = _data[index];
        }
        _data[pos] = value;
    }
}

template <typename T>
void DynamicArray<T>::setAt( const T& value, const size_t pos ) {
    if ( pos > _size ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    _data[pos] = value;
}

template <typename T>
void DynamicArray<T>::removeAt( const size_t pos ) {
    if ( pos > _size || isEmpty() ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    
    for (size_t index = pos; index < _size - 1; index++) {
        _data[index] = _data[index + 1];
    }

    shrink(1);
}

template <typename T>
void DynamicArray<T>::swap( const size_t pos1, const size_t pos2 ) {
    if (pos1 >= _size || pos2 >= _size) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS ); 
    }
    if (pos1 != pos2) {
        auto temp = _data[pos1];
        _data[pos1] = _data[pos2];
        _data[pos2] = temp;
    }
}

template <typename T>
void DynamicArray<T>::extend( int sizeDiff ) {
    if ( sizeDiff < 0 ) {
        throw Exception( Exception::ErrorCode::NEGATIVE_SIZE_DIFFERENCE );
    }

    _size += sizeDiff;
    auto newCapacity = _capacity;

    if ( newCapacity < 10000 ) {
        newCapacity = ( _size > newCapacity * 0.50 ) 
            ? newCapacity * 2
            : newCapacity;
    } else {
        newCapacity = ( _size > newCapacity * 0.25 )
            ? newCapacity + 1000
            : newCapacity;
    }
    if ( newCapacity != _capacity ) {
        _offset = newCapacity / 4 + 1;
        T* newAllocBegin = new T[newCapacity + _offset];
        T* newData = newAllocBegin + _offset;
        T* newAllocEnd = newAllocBegin + (newCapacity + _offset);
        for ( size_t index = 0; index < _size - sizeDiff; index++ ) {
            newData[index] = _data[index];
        }
        delete[] _allocBegin;
        _capacity = newCapacity;
        _allocBegin = newAllocBegin;
        _data = newData;
        _allocEnd = newAllocEnd;    
    }
}

template <typename T>
void DynamicArray<T>::shrink( const int sizeDiff ) {
    if ( sizeDiff < 0 ) {
        throw Exception( Exception::ErrorCode::NEGATIVE_SIZE_DIFFERENCE );
    }

    _size -= sizeDiff;
    auto newCapacity = _capacity;

    if ( newCapacity < 10000 ) {
        newCapacity = ( _size < newCapacity * 0.25 )
            ? newCapacity / 2
            : newCapacity;
    } else {
        newCapacity = ( _size < newCapacity * 0.75 )
            ? newCapacity - 1000
            : newCapacity;
    }
    if ( newCapacity != _capacity ) {
        _offset = newCapacity / 4 + 1;
        T* newAllocBegin = new T[newCapacity + _offset];
        T* newData = newAllocBegin + _offset;
        T* newAllocEnd = newAllocBegin + (newCapacity + _offset);
        for ( size_t index = 0; index < _size; index++ ) {
            newData[index] = _data[index];
        }
        delete[] _allocBegin;
        _capacity = newCapacity;
        _allocBegin = newAllocBegin;
        _data = newData;
        _allocEnd = newAllocEnd;      
    }
}

template <typename T>
DynamicArray<T> DynamicArray<T>::subArray( const size_t start, const size_t end ) const {
    if (start > end || start >= _size || end > _size) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    if (start == end) { return DynamicArray<T>(); }
    DynamicArray<T> res = DynamicArray<T>(end - start);
    for ( size_t index = 0; index < end - start; index++ ) {
        res.append( _data[start + index]);
    }
    return res;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::concat( const DynamicArray<T>& other ) {
    DynamicArray<T>* res = new DynamicArray<T>(_capacity + other._capacity);
    for ( size_t index = 0; index < _size; index++ ) {
        res->append( (*this)[index] );
    }
    for ( size_t index = 0; index < other._size; index++ ) {
        res->append( other[index] );
    }
    return res;
}

template <typename T>
T& DynamicArray<T>::operator[]( const size_t index ) {
    if ( index >= _size ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    return _data[index];
}

template <typename T>
const T& DynamicArray<T>::operator[]( const size_t index ) const {
    if ( index >= _size ) {
        throw Exception( Exception::ErrorCode::INDEX_OUT_OF_BOUNDS );
    }
    return _data[index];
}

template <typename T>
void DynamicArray<T>::clear() {
    _size = 0;
    _offset = 1;
    _capacity = 2;

    delete[] _allocBegin;

    _allocBegin = new T[_offset + _capacity];
    _data = _allocBegin + _offset;
    _allocEnd = _allocBegin + _offset + _capacity;
}

template <typename T>
size_t DynamicArray<T>::getSize() const {
    return _size;
}

template <typename T>
size_t DynamicArray<T>::getCapacity() const {
    return _capacity;
}

template <typename T>
void DynamicArray<T>::map( const std::function<T(T&)>& func ) {
    for (size_t index = 0; index < _size; index++) {
        _data[index] = func( _data[index] );
    }
}

template <typename T>
void DynamicArray<T>::where( const std::function<bool(T)>& func ) {
    for ( size_t index = 0; index < _size; ) {
        if ( !func((*this)[index]) ) { removeAt(index); } 
        else { index++; }
    }
}

template <typename T>
bool DynamicArray<T>::isEmpty() const {
    return _size == 0;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::appendImmutable( const T& value ) const {
    DynamicArray<T>* res = new DynamicArray<T>(*this);
    res->append( value );
    return res;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::prependImmutable( const T& value ) const {
    DynamicArray<T>* res = new DynamicArray<T>(*this);
    res->prepend( value );
    return res;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::insertAtImmutable( const T& value, const size_t pos ) const {
    DynamicArray<T>* res = new DynamicArray<T>(*this);
    res->insertAt( value, pos );
    return res;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::removeAtImmutable( const size_t pos ) const {
    DynamicArray<T>* res = new DynamicArray<T>(*this);
    res->removeAt(pos);
    return res;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::setAtImmutable( const T& value, const size_t pos ) const {
    DynamicArray<T>* res = new DynamicArray<T>(*this);
    res->setAt( value, pos );
    return res;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::swapImmutable(const size_t pos1, const size_t pos2) const {
    DynamicArray<T>* res = new DynamicArray<T>(*this);
    res->swap(pos1, pos2);
    return res;
}

template <typename T>
DynamicArray<T>* DynamicArray<T>::concatImmutable( const DynamicArray<T>& other ) const {
    DynamicArray<T>* res = new DynamicArray<T>(*this);
    return res->concat(other);
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