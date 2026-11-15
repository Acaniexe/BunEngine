#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define BUN_PLATFORM_WINDOWS 1
    #ifndef UNICODE
        #define UNICODE
    #endif
    #ifndef _UNICODE
        #define _UNICODE
    #endif
#elif defined(__APPLE__) && defined(__MACH__)
    #define BUN_PLATFORM_MACOS 1
#elif defined(__linux__)
    #define BUN_PLATFORM_LINUX 1
#else
    #error "BunEngine: Unsupported platform."
#endif

#if defined(__x86_64__) || defined(_M_X64)
    #define BUN_ARCH_X64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define BUN_ARCH_ARM64 1
#endif

#if defined (_MSC_VER)
    #define BUN_COMPILER_MSVC 1
    #define BUN_FORCE_INLINE  __forceinline
    #define BUN_NO_INLINE     __declspec(noinline)
    #define BUN_DEBUG_BREAK   __debugbreak()
#elif defined(__clang__)
    #define BUN_COMPILER_CLANG 1
    #define BUN_FORCE_INLINE   __attribute__((always_inline)) inline
    #define BUN_NO_INLINE      __attribute((noinline))
    #define BUN_DEBUG_BREAK    __builtin_debugtrap()
#elif defined(__GNUC__)
    #define BUN_COMPILER_GCC  1
    #define BUN_FORCE_INLINE  __attribute((always_inline)) inline
    #define BUN_NO_INLINE     __attribute((noinline))
    #define BUN_DEBUG_BREAK   __builtin_trap()
#else
    #error "C4ndyEngine: Unsupported compiler."
#endif

#if defined(BUN_PLATFORM_WINDOWS)
    #ifdef BUN_ENGINE
        #define BUN_API  __declspec(dllexport)
    #else
        #define BUN_API  __declspec(dllimport)
    #endif
#else
    #define BUN_API  __attribute__((visibility("default")))
#endif

#include <cstdint>
#include <cstddef>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;
using usize = size_t;
using isize = ptrdiff_t;

#include <string>
#include <string_view>
#include <memory>
#include <functional>
#include <optional>
#include <variant>
#include <vector>
#include <unordered_map>
#include <array>
#include <cassert>

#define BUN_NON_COPYABLE(ClassName)          \
    ClassName(const ClassName&)       = delete; \
    ClassName& operator=(const ClassName&) = delete;

#define BUN_NON_MOVEABLE(ClassName)          \
    ClassName(ClassName&&)       = delete; \
    ClassName& operator=(ClassName&&) = delete;

#define BUN_NON_COPYABLE_MOVEABLE(ClassName) \
    BUN_NON_COPYABLE(ClassName)               \
    BUN_NON_MOVEABLE(ClassName)

#define BUN_BIT(x)         (1u << (x))
#define BUN_HAS_FLAG(v, f) (((v) & (f)) == (f))

#define BUN_UNUSED(x) (void)(x)

template<typename T, usize N>
constexpr usize ArrayCount(const T (&)[N]) { return N; }

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T>
using WeakRef = std::weak_ptr<T>;

template<typename T, typename... Args>
constexpr  Ref<T> MakeRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
constexpr Scope<T> MakeScope(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}