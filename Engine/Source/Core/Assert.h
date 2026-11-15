#pragma once
#include "Core.h"
#include <cstdio>

namespace Bun::Internal
{
    void AssertionFail(const char* expr, const char* file, int line, const char* msg = nullptr)
    {
        if(msg)
            fprintf(stderr, "[BUN ASSERT]: %s\n Expression: %s\n File        : %s\n Line          : %d\n", msg, expr, file, line);
        else
            fprintf(stderr, "[BUN ASSERT]: Assertion Failed\n  Expression : %s\n  File       : %s\n  Line       : %d\n", expr, file, line);
        BUN_DEBUG_BREAK;
    }
}

#ifdef _DEBUG
    #define BUN_ASSERT(expr) \
        do { if(!(expr)) { Bun::Internal::AssertionFail(#expr, __FILE__, __LINE__); } } while(0)
    
    #define BUN_ASSERT_MSG(expr, msg) \
        do { if(!(expr)) { Bun::Internal::AssertionFail(#expr, __FILE__, __LINE__, msg); } } while(0)
#else
    #define BUN_ASSERT(expr)          do {} while(0)
    #define BUN_ASSERT_MSG(expr, msg) do {} while(0)
#endif