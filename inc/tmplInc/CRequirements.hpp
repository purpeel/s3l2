#ifndef CASSOCIATIVE_H
#define CASSOCIATIVE_H

#include "Ordering.hpp"
#include "Pair.hpp"


template <typename TContainer, typename K, typename V>
concept CAssociative =
    requires( TContainer container, const K& key, const Pair<K,V>& pair ) 
    { 
        requires std::same_as<decltype(container.get(key)),V&> 
              || std::same_as<decltype(container.get(key)),const V&>;
        { container.insert(pair) }  -> std::same_as<TContainer&>; //todo || std::same_as<void>
        { container.remove(key) }   -> std::same_as<TContainer&>; //todo || std::same_as<void>
        { container.contains(key) } -> std::convertible_to<bool>;
        { container.isEmpty() }  -> std::convertible_to<bool>;
        { container.getSize() }  -> std::convertible_to<ssize_t>;
        { container.begin() } -> std::same_as<typename TContainer::TIter>;
        { container.end() } -> std::same_as<typename TContainer::TIter>;
    };

template <typename TContainer, typename K, typename V>
concept CChageableByKey = CAssociative<TContainer,K,V> &&
    requires( TContainer container, const K& key ) {
        { container.get } -> std::same_as<V&>;
    };
 

#endif // CASSOCIATIVE_H