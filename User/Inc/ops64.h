#ifndef OPS64_H
#define OPS64_H

static inline int32_t I32xU32_HI32(int32_t a, uint32_t b)
{
  return (((int64_t)a)*b)>>32;
}

static inline int64_t I64xU32_HI64(int64_t a, uint32_t b)
{
  int32_t alxb_h = I32xU32_HI32(a&0xffffffff, b);
  int64_t ahxb = ((a>>32))*b;
  return ahxb + alxb_h;
}

static inline int32_t I64xU32_MI32(uint64_t a, uint32_t b)
{
  int32_t alxb_h = I32xU32_HI32(a&0xffffffff, b);
  int32_t ahxb_l = (int32_t)((a>>32)*b);
  return ahxb_l + alxb_h;
}

static inline int64_t I32dU32_QSI64(int32_t a, uint32_t b)
{
  int64_t a_ls32 = ((int64_t)a)<<32;
  return a_ls32/b;
}

#define MIN(a, b)     (((a)<(b))?(a):(b))
#define MAX(a, b)     (((a)>(b))?(a):(b))
#define CLIP(v, a, b) MIN(MAX(v, a), b)

#endif
