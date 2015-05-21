// XXX private
#define cru_constructor __attribute__((constructor))

// XXX private
#define likely(x) __builtin_expect(!!(x), true)
#define unlikely(x) __builtin_expect(!!(x), false)

