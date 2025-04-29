#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>

#if defined(__x86_64__)
#define PATCH_SIZE 5
#elif defined(__aarch64__) || defined(__arm__) && defined(__ARM_ARCH_7A__) && !defined(__thumb__)
#define PATCH_SIZE 4
#else
#error "Unsupported architecture"
#endif

union jmp_patch_code {
        unsigned char bytes[PATCH_SIZE];
#if defined(__x86_64__)
        struct {
                unsigned char jmp;
                unsigned int offset;
        } __attribute__((__packed__));
#elif defined(__aarch64__) || defined(__arm__)
        struct {
                unsigned int offset: 24;
                unsigned char jmp;
        };
#endif
} __attribute__((__packed__));

static_assert(sizeof(union jmp_patch_code) == PATCH_SIZE, "jmp_patch_code size mismatch");

static inline void *page_align(void *addr)
{
        return (void *)((unsigned long)addr & ~(getpagesize() - 1));
}

static inline unsigned int jmp_offset(void *function, const void *new_function)
{
#if defined(__x86_64__)
        return (unsigned long)new_function - (unsigned long)function - PATCH_SIZE;
#elif defined(__aarch64__)
        return 0xFFFFFF - ((unsigned int)(((unsigned long)function - (unsigned long)new_function - PATCH_SIZE) >> 2) & 0xFFFFFF);
#elif defined(__arm__)
        return (unsigned int)(((unsigned long)new_function - (unsigned long)function - 8) >> 2) & 0xFFFFFF;
#endif
}

static inline unsigned char jmp_code(void *function, const void *new_function)
{
#if defined(__x86_64__)
        return 0xE9; // jmp <offset>
#elif defined(__aarch64__)
        return new_function < function ? 0x17 : 0x14;
#elif defined(__arm__)
        return 0xEA; // b <offset>
#endif
}

__attribute__((nonnull(1, 2)))
int patch_function(void *function, const void *new_function, unsigned char unpatch_code[static PATCH_SIZE])
{
        int err;
        union jmp_patch_code patch_code;
        unsigned char *patch = (void *)function;

        err = mprotect(page_align(function), getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
        if (err < 0) {
                return -1;
        }

        memcpy(unpatch_code, function, PATCH_SIZE);

        patch_code.jmp = jmp_code(function, new_function);
        patch_code.offset = jmp_offset(function, new_function);
        memcpy(patch, patch_code.bytes, PATCH_SIZE);

        err = mprotect(page_align(function), getpagesize(), PROT_READ | PROT_EXEC);
        if (err < 0) {
                return -1;
        }

        return 0;
}

__attribute__((nonnull(1, 2)))
int unpatch_function(void *function, const unsigned char unpatch_code[static PATCH_SIZE])
{
        int err;
        
        err = mprotect(page_align(function), getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
        if (err < 0) {
                return -1;
        }

        memcpy(function, unpatch_code, PATCH_SIZE);

        err = mprotect(page_align(function), getpagesize(), PROT_READ | PROT_EXEC);
        if (err < 0) {
                return -1;
        }

        return 0;
}
