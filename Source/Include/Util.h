#define low_16(address) (short)((address) & 0xFFFF)
#define high_16(address) (short)(((address) >> 16) & 0xFFFF)
#define NULL ((void*)0)
typedef void (*fn_ptr)();