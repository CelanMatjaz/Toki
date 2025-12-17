#pragma once

#include <toki/core/types.h>

namespace toki {

template <b8 Value>
struct BoolConstant {
	static constexpr b8 value = Value;
};

static_assert(BoolConstant<true>::value);
static_assert(!BoolConstant<false>::value);

template <typename T>
concept CHasValueField = requires { T::value; };

static_assert(CHasValueField<BoolConstant<true>>);
static_assert(!CHasValueField<int>);

struct TrueType : BoolConstant<true> {};
struct FalseType : BoolConstant<false> {};

template <typename T>
struct TypeTrueType : BoolConstant<true> {};

template <typename T>
struct TypeFalseType : BoolConstant<false> {};

template <typename T1, typename T2>
struct IsSame : FalseType {};

template <typename T>
struct IsSame<T, T> : TrueType {};

template <typename T1, typename T2>
concept CIsSame = IsSame<T1, T2>::value;

static_assert(IsSame<i32, i32>::value);
static_assert(!IsSame<i32, u32>::value);

template <typename T>
struct RemoveConst {
	using type = T;
};

template <typename T>
struct RemoveConst<const T> {
	using type = RemoveConst<T>::type;
};

template <typename T>
struct RemoveConst<const T*> {
	using type = RemoveConst<T>::type*;
};

template <typename T>
struct RemoveConst<const T&> {
	using type = RemoveConst<T>::type&;
};

template <typename T>
struct RemoveConst<const volatile T> {
	using type = volatile RemoveConst<T>::type;
};

static_assert(IsSame<RemoveConst<u32>::type, u32>::value);
static_assert(IsSame<RemoveConst<const u32>::type, u32>::value);
static_assert(IsSame<RemoveConst<const u32*>::type, u32*>::value);
static_assert(IsSame<RemoveConst<const u32&>::type, u32&>::value);

template <typename T>
struct RemoveRef {
	using type = T;
};

template <typename T>
struct RemoveRef<T&> {
	using type = T;
};

template <typename T>
struct RemoveRef<T&&> {
	using type = T;
};

static_assert(IsSame<RemoveRef<u32>::type, u32>::value);
static_assert(IsSame<RemoveRef<u32&>::type, u32>::value);
static_assert(IsSame<RemoveRef<u32&&>::type, u32>::value);

template <typename T>
struct RemoveVolatile {
	using type = T;
};

template <typename T>
struct RemoveVolatile<volatile T> {
	using type = T;
};

static_assert(IsSame<RemoveVolatile<u32>::type, u32>::value);
static_assert(IsSame<RemoveVolatile<volatile u32>::type, u32>::value);

template <typename T>
struct RemoveCV {
	using type = T;
};

template <typename T>
struct RemoveCV<const T> {
	using type = T;
};

template <typename T>
struct RemoveCV<volatile T> {
	using type = T;
};

template <typename T>
struct RemoveCV<const volatile T> {
	using type = T;
};

static_assert(IsSame<RemoveCV<u32>::type, u32>::value);
static_assert(IsSame<RemoveCV<const u32>::type, u32>::value);
static_assert(IsSame<RemoveCV<volatile u32>::type, u32>::value);
static_assert(IsSame<RemoveCV<const volatile u32>::type, u32>::value);

template <typename T>
struct RemovePointer {
	using type = T;
};

template <typename T>
struct RemovePointer<T*> {
	using type = T;
};

static_assert(IsSame<RemovePointer<u32>::type, u32>::value);
static_assert(IsSame<RemovePointer<u32*>::type, u32>::value);

template <typename T>
struct IsReference : FalseType {
	using ref_type = T;
};

template <typename T>
struct IsReference<T&> : TrueType {
	using ref_type = T&;
};

template <typename T>
struct IsReference<T&&> : TrueType {
	using ref_type = T&&;
};

template <typename T>
concept CIsReference = IsReference<T>::value;

static_assert(!IsReference<u32>::value);
static_assert(IsReference<u32&>::value);
static_assert(IsReference<u32&&>::value);

template <typename T>
struct IsRValueReference : FalseType {};

template <typename T>
struct IsRValueReference<T&&> : TrueType {};

template <typename T>
concept CIsRValueReference = IsRValueReference<T>::value;

static_assert(!IsRValueReference<u32>::value);
static_assert(!IsRValueReference<u32&>::value);
static_assert(IsRValueReference<u32&&>::value);

template <typename T>
struct IsLValueReference : FalseType {};

template <typename T>
struct IsLValueReference<T&> : TrueType {};

template <typename T>
concept CIsLValueReference = IsLValueReference<T>::value;

static_assert(!CIsLValueReference<u32>);
static_assert(CIsLValueReference<u32&>);
static_assert(!CIsLValueReference<u32&&>);

template <typename T>
	struct IsIntegral : BoolConstant < requires(T t, T* p, void (*f)(T)) {
	reinterpret_cast<T>(t);
	f(0);
	p + t;
}>{};

template <typename T>
concept CIsIntegral = IsIntegral<T>::value;

static_assert(IsIntegral<i32>::value);
static_assert(!IsIntegral<f32>::value);

template <typename T>
struct IsFloatingPoint : BoolConstant<IsSame<T, f32>::value || IsSame<T, f64>::value || IsSame<T, f128>::value> {};

template <typename T>
concept CIsFloatingPoint = IsFloatingPoint<T>::value;

static_assert(IsFloatingPoint<f32>::value);
static_assert(IsFloatingPoint<f64>::value);
static_assert(IsFloatingPoint<f128>::value);
static_assert(!IsFloatingPoint<i32>::value);

template <typename T, T val>
	requires CIsIntegral<T>
struct IntegralConstant {
	using type				 = IntegralConstant;
	static constexpr T value = val;
};

static_assert(IntegralConstant<i32, 0>::value == 0);

template <typename T1, typename T2>
struct IsDifferent {
	static constexpr b8 value = !IsSame<T1, T2>::value;
};

template <typename T1, typename T2>
concept CIsDifferent = IsDifferent<T1, T2>::value;

static_assert(!IsDifferent<i32, i32>::value);
static_assert(IsDifferent<i32, u32>::value);

template <b8 Condition, typename T1, typename T2>
struct Conditional : BoolConstant<Condition> {
	using type = T1;
};

template <typename T1, typename T2>
struct Conditional<false, T1, T2> : FalseType {
	using type = T2;
};

static_assert(IsSame<u32, Conditional<true, u32, u64>::type>::value);
static_assert(Conditional<true, u32, u64>::value);
static_assert(IsSame<u64, Conditional<false, u32, u64>::type>::value);
static_assert(!Conditional<false, u32, u64>::value);

template <typename From, typename To>
concept CIsConvertible = requires(From from, To to) { static_cast<To>(from); };

template <typename From, typename To>
struct IsConvertible : BoolConstant<CIsConvertible<From, To>> {};

static_assert(IsConvertible<char*, void*>::value);
static_assert(!IsConvertible<char, void*>::value);
static_assert(IsConvertible<TrueType, BoolConstant<true>>::value);

template <typename T, b8 DefaultValue = false>
struct ConvertedBoolValue : Conditional<CHasValueField<T>, T, BoolConstant<DefaultValue>>::type {};

static_assert(ConvertedBoolValue<TrueType, false>::value);
static_assert(ConvertedBoolValue<TrueType, true>::value);
static_assert(!ConvertedBoolValue<FalseType, false>::value);
static_assert(!ConvertedBoolValue<FalseType, true>::value);
static_assert(!ConvertedBoolValue<i32, false>::value);
static_assert(ConvertedBoolValue<i32, true>::value);

template <typename...>
struct Disjunction : TrueType {};

template <typename T>
	requires CHasValueField<T>
struct Disjunction<T> : T {};

template <typename T, typename... Args>
	requires CHasValueField<T>
struct Disjunction<T, Args...> : Conditional<b8(T::value), TrueType, Disjunction<Args...>>::type {};

template <typename... Args>
concept CDisjunction = Disjunction<Args...>::value;

static_assert(Disjunction<TrueType, TrueType>::value);
static_assert(Disjunction<TrueType, FalseType>::value);
static_assert(Disjunction<FalseType, TrueType>::value);
static_assert(!Disjunction<FalseType, FalseType>::value);

static_assert(Disjunction<FalseType, FalseType, FalseType, FalseType, FalseType, TrueType>::value);
static_assert(Disjunction<TrueType, TrueType, TrueType, TrueType, TrueType, FalseType>::value);
static_assert(!Disjunction<FalseType, FalseType, FalseType, FalseType, FalseType, FalseType>::value);

template <typename...>
struct Conjunction : FalseType {};

template <typename T>
	requires CHasValueField<T>
struct Conjunction<T> : T {};

template <typename T, typename... Args>
	requires CHasValueField<T>
struct Conjunction<T, Args...> : Conditional<b8(T::value), Conjunction<Args...>, FalseType>::type {};

template <typename... Args>
concept CConjunction = Conjunction<Args...>::value;

static_assert(Conjunction<TrueType, TrueType>::value);
static_assert(!Conjunction<TrueType, FalseType>::value);
static_assert(!Conjunction<FalseType, TrueType>::value);
static_assert(!Conjunction<FalseType, FalseType>::value);

static_assert(!Conjunction<FalseType, FalseType, FalseType, FalseType, FalseType, TrueType>::value);
static_assert(!Conjunction<TrueType, TrueType, TrueType, TrueType, TrueType, FalseType>::value);
static_assert(Conjunction<TrueType, TrueType, TrueType, TrueType, TrueType, TrueType>::value);

template <typename T>
	requires CHasValueField<T>
struct Negation : BoolConstant<!(T::value)> {};

static_assert(!Negation<TrueType>::value);
static_assert(Negation<FalseType>::value);

template <typename T>
struct IsSigned : IntegralConstant<b8, T(-1) < T(0)> {};

template <typename T>
concept CIsSigned = IsSigned<T>::value;

static_assert(IsSigned<i32>::value);
static_assert(!IsSigned<u32>::value);
static_assert(IsSigned<f32>::value);

template <typename T>
struct IsPointer : FalseType {};

template <typename T>
struct IsPointer<T*> : TrueType {};

template <typename T>
struct IsPointer<const T*> : TrueType {};

template <typename T>
struct IsPointer<volatile T*> : TrueType {};

template <typename T>
struct IsPointer<const volatile T*> : TrueType {};

template <typename T>
concept CIsPointer = IsPointer<T>::value;

static_assert(IsPointer<void*>::value);
static_assert(IsPointer<const void*>::value);
static_assert(IsPointer<volatile void*>::value);
static_assert(IsPointer<const volatile void*>::value);
static_assert(IsPointer<u32*>::value);
static_assert(!IsPointer<u32>::value);

template <typename T>
struct IsConst : FalseType {};

template <typename T>
struct IsConst<const T> : TrueType {};

template <typename T>
concept CIsConst = IsConst<T>::value;

static_assert(!CIsConst<i32>);
static_assert(CIsConst<const i32>);

template <typename T>
struct IsCArray : FalseType {};

template <typename T>
struct IsCArray<T[]> : TrueType {
	using type				   = T;
	static constexpr u64 COUNT = sizeof(T) / sizeof(RemovePointer<T>::type);
};

template <typename T, u64 N>
struct IsCArray<T[N]> : TrueType {
	using type				   = T;
	static constexpr u64 COUNT = N;
};

template <typename T>
concept CIsCArray = IsCArray<T>::value;

template <typename T>
concept AllocatorConcept = requires(T a, u64 size, void* free_ptr) {
	{ a.allocate(size) } -> CIsSame<void*>;
	{ a.free(free_ptr) } -> CIsSame<void>;
};

template <typename T>
concept CToStringFunctionExists = requires(T t, char* buf) {
	{ to_string(buf, t) } -> CIsSame<u32>;
};

template <typename T1, typename T2>
concept CIsSameWithoutConst = IsSame<typename RemoveConst<T1>::type, typename RemoveConst<T2>::type>::value;

static_assert(CIsSameWithoutConst<const char*, char*>);
static_assert(CIsSameWithoutConst<const volatile char*, volatile char*>);

template <typename T>
concept CHasDestructor = requires(T t) { t.~T(); };

template <typename Allocator>
concept CIsAllocator = requires(u64 size, void* ptr, u64 alignment) {
	{ Allocator::allocate(size) } -> CIsSame<void*>;
	{ Allocator::allocate_aligned(size, alignment) } -> CIsSame<void*>;
	{ Allocator::free(ptr) } -> CIsSame<void>;
	{ Allocator::free_aligned(ptr) } -> CIsSame<void>;
	{ Allocator::reallocate(ptr, size) } -> CIsSame<void*>;
	{ Allocator::reallocate_aligned(ptr, size, alignment) } -> CIsSame<void*>;
};

template <typename T>
struct StringDumper;

template <typename Type, typename Container = StringDumper<typename RemoveCV<Type>::type>>
concept CHasDumpToString = requires(Container c, const Type t, char* out) {
	{ Container::dump_to_string(out, t) } -> CIsSame<u64>;
};

template <typename Container, typename FromType, typename ToType>
concept CConvertTo = requires(FromType from) {
	{ Container::template convert_to<ToType>(from) } -> CIsSame<ToType>;
};

template <typename ToType, typename FromType>
concept CHasConvertTo = requires(const FromType& from) {
	{ convert_to(from) } -> CIsSame<ToType>;
};

template <typename T, typename Container>
concept CIsArrayContainer = requires(const Container container) {
	{ container.data() } -> CIsSame<const T*>;
	{ container.size() } -> CIsSame<u64>;
};

template <u64... Is>
struct IndexSequence {};

template <u64 N, u64... Is>
struct MakeIndexSequenceHelper : MakeIndexSequenceHelper<N - 1, N - 1, Is...> {};

template <u64... Is>
struct MakeIndexSequenceHelper<0, Is...> {
	using type = IndexSequence<Is...>;
};

template <u64 N>
using MakeIndexSequence = typename MakeIndexSequenceHelper<N>::type;

template <typename Callable, typename ReturnType, typename... Args>
concept CIsCorrectCallable = requires(Callable fn, Args... args) {
	{ fn(args...) } -> CIsSame<ReturnType>;
};

template <typename ReturnType, typename... Args>
concept CIsCorrectFn = requires(ReturnType (*fn)(Args...), Args... args) {
	{ fn(args...) } -> CIsSame<ReturnType>;
};

}  // namespace toki
