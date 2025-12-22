#ifndef PAIR_H
#define PAIR_H

#include <type_traits>
#include "util.hpp"

template <typename T1, typename T2>
class Pair
{
public:
    template <typename U1, typename U2> 
    requires (std::constructible_from<T1,U1&&> && std::constructible_from<T2,U2&&>)
    Pair( U1&& value1, U2&& value2 ) : _value1(std::forward(value1)), _value2(std::forward(value2)) {}
    
    template <typename U1, typename U2> 
    requires (std::constructible_from<T1,U1 const&> && std::constructible_from<T2,U2 const&>)
    Pair( const U1& value1, const U2& value2 ) : _value1(value1), _value2(value2) {}

    Pair() = default;
    Pair( const Pair<T1,T2>& other ) = default;
    Pair<T1,T2>& operator=( const Pair<T1,T2>& other ) = default;
    Pair( Pair<T1,T2>&& other ) = default;
    Pair<T1,T2>& operator=( Pair<T1,T2>&& other ) = default;

    ~Pair() = default;
public: 
    T1& first() noexcept { return _value1; }
    const T1& first() const noexcept { return _value1; }
    
    T2& second() noexcept { return _value2; }
    const T2& second() const noexcept { return _value2; }
    
    void swap( Pair<T1,T2>& other ) {
        using std::swap;
        swap( _value1, other._value1 );
        swap( _value2, other._value2 );
    }
public: // structured binding get()
    template <size_t I> const auto& get() const {
        if constexpr (I == 1) return _value1;
        else return _value2;
    }
    template <size_t I> auto& get() {
        if constexpr (I == 1) return _value1;
        else return _value2;
    }

    friend bool operator==( const Pair<T1,T2>& lhs, const Pair<T1,T2>& rhs ) {
        return lhs._value1 == rhs._value1 && lhs._value2 == rhs._value2;
    }
    friend bool operator!=( const Pair<T1,T2>& lhs, const Pair<T1,T2>& rhs ) {
        return !(lhs == rhs);
    }
private:
    T1 _value1;
    T2 _value2;
};

template <typename T1, typename T2>
struct std::tuple_size<Pair<T1,T2>> : std::integral_constant<std::size_t, 2> {};

template <typename T1, typename T2>
struct std::tuple_element<0, Pair<T1,T2>> { using type = T1; };
template <typename T1, typename T2>
struct std::tuple_element<1, Pair<T1,T2>> { using type = T2; };


#endif // PAIR_H