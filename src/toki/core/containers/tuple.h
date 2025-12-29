#pragma once

#include <toki/core/common/type_traits.h>
#include <toki/core/types.h>

#include "toki/core/common/common.h"

namespace toki {

template <u64 I, typename... Ts>
struct NthType;

template <u64 I, typename T, typename... Ts>
struct NthType<I, T, Ts...> {
	using type = typename NthType<I - 1, Ts...>::type;
};

template <typename T, typename... Ts>
struct NthType<0, T, Ts...> {
	using type = T;
};

template <u64 Index, typename T>
struct TupleLeaf {
	static constexpr u64 index = Index;
	T value;

	constexpr TupleLeaf() = default;
	template <class U>
	constexpr TupleLeaf(U&& v): value(toki::forward<U>(v)) {}
};

template <typename IndexSeq, typename... Ts>
struct TupleImpl;

template <u64... Is, typename... Ts>
struct TupleImpl<IndexSequence<Is...>, Ts...> : TupleLeaf<Is, Ts>... {
	constexpr TupleImpl() = default;

	template <class... Us>
	constexpr TupleImpl(Us&&... args): TupleLeaf<Is, Ts>(toki::forward<Us>(args))... {}
};

template <typename... Ts>
struct Tuple : TupleImpl<MakeIndexSequence<sizeof...(Ts)>, Ts...> {
	static constexpr u64 size = sizeof...(Ts);

	constexpr Tuple() = default;

	constexpr Tuple(Ts&&... args): TupleImpl<MakeIndexSequence<sizeof...(Ts)>, Ts...>(toki::forward<Ts>(args)...) {}
};

#define TUPLE_GET(CONST, REF)                                         \
	template <u64 I, typename... Ts>                                  \
	constexpr decltype(auto) get(CONST Tuple<Ts...> REF t) noexcept { \
		using Leaf = TupleLeaf<I, typename NthType<I, Ts...>::type>;  \
		return (static_cast<CONST Leaf REF>(t).value);                \
	}

TUPLE_GET(, &)
TUPLE_GET(, &&)
TUPLE_GET(const, &)
TUPLE_GET(const, &&)

#undef TUPLE_GET

template <typename... Ts>
Tuple<Ts...> make_tuple(Ts&&... args) {
	return Tuple<Ts...>{ static_cast<Ts&&>(args)... };
}

template <typename Head, typename... Tail>
struct TupleStack {
	Head value;
	TupleStack<Tail...> tail;
};

template <typename Tail>
struct TupleStack<Tail> {
	Tail value;
};

}  // namespace toki
