#include <cstdint>
#include <cstring>

static inline int JSONStringCopyInline (uint8_t*& dst, uint8_t const*& src,
                                        int limit) {
  // Copy up to limit uint8_t from src to dst.
  // Stop at the first control character or backslash or double quote.
  // Report the number of bytes copied. May copy less bytes, for example
  // for alignment reasons.
  int count = limit;
  while (count > 0 && 
         *src >= 32 && 
         *src != '\\' && 
         *src != '"') {
    *dst++ = *src++;
    count--;
  }
  return limit - count;
}

int JSONStringCopyC (uint8_t*& dst, uint8_t const*& src, int limit) {
  return JSONStringCopyInline(dst, src, limit);
}

extern int (*JSONStringCopy)(uint8_t*&, uint8_t const*&, int);

#if defined(__SSE4_2__) && ! defined(NO_SSE42)

#include <x86intrin.h>

static int JSONStringCopySSE42 (uint8_t*& dst, uint8_t const*& src, int limit) {
  if (limit < 16) {
    return JSONStringCopyInline(dst, src, limit);
  }
  // Align src to 16 byte boundaries:
  int lim = 0xf - ((reinterpret_cast<uintptr_t>(src)-1) & 0xf);
  int count = 0;
  if (lim > 0) {
    count = JSONStringCopyInline(dst, src, lim);
    if (count < lim) {
      return count;
    }
    limit -= count;
  }
  // Now src is aligned on 16 byte boundaries
  alignas(16) static char const ranges[16] = "\x00\x1f\"\"\\\\";
  __m128i const r = _mm_load_si128(reinterpret_cast<__m128i const*>(ranges));
  while (limit >= 16) {
    __m128i const s = _mm_load_si128(reinterpret_cast<__m128i const*>(src));
    int const x = _mm_cmpestri(r,6, s, 16,
                               _SIDD_UBYTE_OPS | _SIDD_CMP_RANGES |
                               _SIDD_POSITIVE_POLARITY |
                               _SIDD_LEAST_SIGNIFICANT);
    if (x < 16) {
      memcpy(dst, src, x);
      dst += x;
      src += x;
      count += x;
      return count;
    }
    _mm_storeu_si128(reinterpret_cast<__m128i*>(dst), s);
    src += 16;
    dst += 16;
    limit -= 16;
    count += 16;
  }
  count += JSONStringCopyInline(dst, src, limit);
  return count;
}

static int DoInit (uint8_t*& dst, uint8_t const*& src, int limit) {
  if (__builtin_cpu_supports("sse4.2")) {
    JSONStringCopy = JSONStringCopySSE42;
  }
  else {
    JSONStringCopy = JSONStringCopyC;
  }
  return (*JSONStringCopy)(dst, src, limit);
}

#else

static int DoInit (uint8_t*& dst, uint8_t const*& src, int limit) {
  JSONStringCopy = JSONStringCopyC;
  return JSONStringCopyC(dst, src, limit);
}

#endif

