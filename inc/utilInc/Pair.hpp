#ifndef PAIR_H
#define PAIR_H

#include <concepts>
#include <type_traits>
#include "util.hpp"

template <typename Base, typename Derived> 
concept DerivedFrom = std::is_base_of_v<Base, Derived>;

template <typename T1, typename T2>
class Pair
{
public:
    Pair() = default;

    template <typename D1, typename D2> 
    requires (DerivedFrom<T1,D1> && DerivedFrom<T2,D2>)
    Pair( const D1& value1, const D2& value2 ) {
        new(&_value1) D2( value1 );
        new(&_value2) D2( value2 );
    }

    Pair( const Pair<T1,T2>& other ) : _value1( other._value1 ), _value2( other._value2 ) {}
    Pair<T1,T2>& operator=( const Pair<T1,T2>& other ) {
        if ( this != &other ) {
            _value1 = other._value1;
            _value2 = other._value2;
        }
        return *this;
    }

    Pair( Pair<T1,T2>&& other ) : _value1( std::move( other._value1 ) ), _value2( std::move( other._value2 ) ) {}
    Pair<T1,T2>& operator=( Pair<T1,T2>&& other )  {
        if ( this != &other ) {
            _value1 = std::move( other._value1 );
            _value2 = std::move( other._value2 );
        }
        return *this;
    }

    ~Pair() = default;
public:
    T1& getT1() { return _value1; }
    const T1& getT1() const { return _value1; }
    
    T2& getT2() { return _value2; }
    const T2& getT2() const { return _value2; }
private:
    T1 _value1;
    T2 _value2;
};

#endif // PAIR_H