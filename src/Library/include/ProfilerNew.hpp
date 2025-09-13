// src/Library/include/ProfilerNew.hpp
#pragma once
#include "Callsite.hpp"
#include <typeinfo>

#define MP_NEW_FT(T, ...) ( mp::ScopedCallsite(__FILE__, __LINE__, typeid(T).name()), new T(__VA_ARGS__) )

#define MP_SET_CALLSITE()   ::mp::setCallsite(__FILE__, __LINE__)
#define MP_SET_TYPENAME(T)  ::mp::setTypeName(typeid(T).name())
