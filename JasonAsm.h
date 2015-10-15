#include <stdint.h>

static inline int JSONStringCopyInline (uint8_t*& dst, uint8_t const*& src,
                                        int limit) {
  // Copy up to limit uint8_t from src to dst.
  // Stop at the first control character or backslash or double quote.
  // Report the number of bytes copied.
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
  return JsonStringCopyInline(dst, src, limit);
}

extern int (*JSONStringCopy)(uint8_t*&, uint8_t const*&, int);

#ifdef USE_SSE42 &&__SSE4_2

#include <nmmintrin.h>

static int JSONStringCopySSE42 (uint8_t*& dst, uint8_t*& src, int limit) {
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
  static char const ranges[16] = "\x00\x1f\"\"\\\\";
  __m128i const r = _mm_loadu_si128(reinterpret_cast<__m123i const*>(ranges));
  lim = limit;
  while (limit >= 16) {
    __m128i const s = _mm_load_si128
    src += 16;
    dst += 16;
    limit -= 16;
  }
  return count + lim - limit;



}

static int DoInit (uint8_t* dst, uint8_t* src, int limit) {
  if (__builtin_cpu_supports(sse4.2)) {
    JSONStringCopy = JSONStringCopySSE42;
  }
  else {
    JSONStringCopy = JSONStringCopyC;
  }
  return (*JsonStringCopy)(dst, src, limit);
}

#else

static int DoInit (uint8_t* dst, uint8_t* src, int limit) {
  JSONStringCopy = JSONStringCopyC;
  return JsonStringCopyC(dst, src, limit);
}

#endif

int (*JSONStringCopy)(uint8_t*, uint8_t const*, int) = DoInit;

