// src/Library/include/ProfilerNew.hpp
#pragma once
#include <typeinfo>
#include "Callsite.hpp"

// --- USO BÁSICO (sin metadatos) ---
#define MP_NEW(T, ...)               new T(__VA_ARGS__)
#define MP_NEW_ARRAY(T, N)           new T[N]

// --- CAPTURA __FILE__ y __LINE__ (una sola expresión gracias a lambda) ---
#define MP_NEW_F(T, ...) \
    ([&](){ ::mp::ScopedCallsite _mp_sc(__FILE__, __LINE__); return new T(__VA_ARGS__); }())

#define MP_NEW_ARRAY_F(T, N) \
    ([&](){ ::mp::ScopedCallsite _mp_sc(__FILE__, __LINE__); return new T[N]; }())

// --- CAPTURA type_name (posiblemente mangleado) ---
#define MP_NEW_T(T, ...) \
    ([&](){ ::mp::ScopedCallsite _mp_sc(nullptr, 0, typeid(T).name()); return new T(__VA_ARGS__); }())

#define MP_NEW_ARRAY_T(T, N) \
    ([&](){ ::mp::ScopedCallsite _mp_sc(nullptr, 0, typeid(T).name()); return new T[N]; }())

// --- CAPTURA TODO: file, line y type_name (RECOMENDADO) ---
#define MP_NEW_FT(T, ...) \
    ([&](){ ::mp::ScopedCallsite _mp_sc(__FILE__, __LINE__, typeid(T).name()); return new T(__VA_ARGS__); }())

#define MP_NEW_ARRAY_FT(T, N) \
    ([&](){ ::mp::ScopedCallsite _mp_sc(__FILE__, __LINE__, typeid(T).name()); return new T[N]; }())
