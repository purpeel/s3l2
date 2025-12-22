#ifndef BTREE_DETAILS_H
#define BTREE_DETAILS_H

#include <concepts>

template <typename K>
concept COrdered = requires( K arg1, K arg2 ) { 
    { arg1  < arg2 } -> std::convertible_to<bool>;
    { arg1 == arg2 } -> std::convertible_to<bool>;
};

#endif // BTREE_DETAILS_H