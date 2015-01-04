#if !defined(COROUTINES_H)

#define CORO_INITIALIZED 357407

#define CORO_BEGIN if (context->jmp == CORO_INITIALIZED) { CoroutineContext c = {}; *context = c; } \
                   switch(context->jmp) { case 0:
#define YIELD(x) do { context->jmp = __LINE__; return x; \
                         case __LINE__:; } while (0)
#define CORO_END } CoroutineContext c = {}; *context = c; // reset context

//#define COROUTINE(x) x(CoroutineContext *context)
#define CORO_STACK(vars) struct STACK##__LINE__ { vars }; \
                         Assert(sizeof(STACK##__LINE__) <= sizeof(context->__PADDING__)); \
                         STACK##__LINE__ *stack = (STACK##__LINE__ *)context->__PADDING__

struct CoroutineContext
{
    uint32 jmp;
    uint8 __PADDING__[1024];
};

#define COROUTINES_H
#endif
