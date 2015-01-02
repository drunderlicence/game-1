#if !defined(COROUTINES_H)

#define crBegin if (!context->jmp) { context->jmp = 0; } switch(context->jmp) { case 0:
#define crReturn(x) do { context->jmp = __LINE__; return x; \
                         case __LINE__:; } while (0)
#define crFinish } context->jmp = 0;

#define crStackBegin typedef struct {
#define crStackEnd(x) } x; Assert(sizeof(x) <= sizeof(context->__PADDING__)); x *stack = (x *)context->__PADDING__

struct CoroutineContext
{
    uint32 jmp;
    uint8 __PADDING__[1024];
};

#define COROUTINES_H
#endif
