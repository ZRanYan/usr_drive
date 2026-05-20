#ifndef NO_OPT_H
#define NO_OPT_H

/* ==== GCC / Clang ==== */
#if defined(__GNUC__) && !defined(__clang__)
  #define NOOPT_PUSH _Pragma("GCC push_options")
  #define NOOPT_POP  _Pragma("GCC pop_options")
  #define NOOPT_OFF  _Pragma("GCC optimize(\"O0\")")
  /* 用于单个函数前：禁止优化且禁止内联 */
  #define NO_OPT_FUNC __attribute__((optimize("O0"), noinline))
#elif defined(__clang__)
  /* clang 支持类似的 pragma，但 attribute(optimize) 也常可用 */
  #define NOOPT_PUSH _Pragma("GCC push_options")
  #define NOOPT_POP  _Pragma("GCC pop_options")
  #define NOOPT_OFF  _Pragma("GCC optimize(\"O0\")")
  #define NO_OPT_FUNC __attribute__((optnone)) /* clang 专用，等同于 O0 + noinline */
#elif defined(_MSC_VER)
  /* MSVC */
  #define NOOPT_PUSH __pragma(optimize("", off))
  #define NOOPT_POP  __pragma(optimize("", on))
  #define NOOPT_OFF  /* push already disables */
  #define NO_OPT_FUNC __declspec(noinline)
#else
  #define NOOPT_PUSH
  #define NOOPT_POP
  #define NOOPT_OFF
  #define NO_OPT_FUNC
#endif

/* 编译器屏障（禁止编译器重排序、擦除） */
#if defined(__GNUC__) || defined(__clang__)
  #define COMPILER_BARRIER() asm volatile("" ::: "memory")
#else
  #include <stdatomic.h>
  #define COMPILER_BARRIER() atomic_thread_fence(memory_order_seq_cst)
#endif

/* 简单的 MMIO 访问宏（示意） */
// #include <stdint.h>
// #define MMIO32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))

#endif /* NO_OPT_H */
