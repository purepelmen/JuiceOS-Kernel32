#pragma once
/*
    Provides utilities related to language features (C/C++).
    Special things like __init_array_start/__init_array_end defined in the ld script.
*/

typedef void (*g_ctor)();

extern "C" g_ctor __init_array_start;
extern "C" g_ctor __init_array_end;

void cpp_call_global_ctors()
{
    for (g_ctor* i = &__init_array_start; i != &__init_array_end; i++)
    {
        (*i)();
    }
}
