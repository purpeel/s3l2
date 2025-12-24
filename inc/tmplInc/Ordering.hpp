#ifndef ORDERING_H
#define ORDERING_H

#include <concepts>

template <typename K>
concept COrdered = requires( const K& arg1, const K& arg2 ) { 
    { arg1  < arg2 } -> std::convertible_to<bool>;
    { arg1 == arg2 } -> std::convertible_to<bool>;
};

#endif // BTREE_DETAILS_H