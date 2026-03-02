#pragma once
#include <ntddk.h>
#include <impl/obf/obf.hxx>
#ifndef OBF_SALT
#define OBF_SALT 0x1337u
#endif

// Safe volatile dummy calculation (no memory access)
#define VOLATILE_CALC(x) do { \
    volatile unsigned __tmp = (x); \
    __tmp ^= 0xDEADBEEF; \
    __tmp += 0xCAFEBABE; \
} while(0)

// One safe bogus block
#define INSERT_BOGUS_BLOCK(seed) do {        \
    unsigned __obf = (seed) ^ OBF_SALT;     \
    VOLATILE_CALC(__obf);                    \
} while(0)

#define INSERT_16_BOGUS() \
    INSERT_BOGUS_BLOCK(__COUNTER__); INSERT_BOGUS_BLOCK(__COUNTER__); \
    INSERT_BOGUS_BLOCK(__COUNTER__); INSERT_BOGUS_BLOCK(__COUNTER__); \
    INSERT_BOGUS_BLOCK(__COUNTER__); INSERT_BOGUS_BLOCK(__COUNTER__); \
    INSERT_BOGUS_BLOCK(__COUNTER__); INSERT_BOGUS_BLOCK(__COUNTER__); \
    INSERT_BOGUS_BLOCK(__COUNTER__); INSERT_BOGUS_BLOCK(__COUNTER__); \
    INSERT_BOGUS_BLOCK(__COUNTER__); INSERT_BOGUS_BLOCK(__COUNTER__); \
    INSERT_BOGUS_BLOCK(__COUNTER__); INSERT_BOGUS_BLOCK(__COUNTER__); \
    INSERT_BOGUS_BLOCK(__COUNTER__); INSERT_BOGUS_BLOCK(__COUNTER__);

#define INSERT_64_BOGUS() \
    INSERT_16_BOGUS(); INSERT_16_BOGUS(); INSERT_16_BOGUS(); INSERT_16_BOGUS();

#define INSERT_256_BOGUS() \
    INSERT_64_BOGUS(); INSERT_64_BOGUS(); INSERT_64_BOGUS(); INSERT_64_BOGUS();

// Emit multiple blocks
#define INSERT_HEAVY_BOGUS() do {            \
    for (int i = 0; i < 1; i++)            \
        INSERT_BOGUS_BLOCK(__COUNTER__ + i);\
} while(0)

// Kernel-safe wrappers
#define OBFUSCATE_BEGIN do { \
    INSERT_256_BOGUS();   \
    if (TRUE) {

#define OBFUSCATE_END \
    }                       \
    INSERT_256_BOGUS();   \
} while(0)