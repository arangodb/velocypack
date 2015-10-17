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

static inline int JSONSkipWhiteSpaceInline (uint8_t const*& ptr, int limit) {
  // Skip up to limit uint8_t from ptr as long as they are whitespace.
  // Advance ptr and return the number of skipped bytes.
  int count = limit;
  while (count > 0 &&
         (*ptr == ' ' ||
          *ptr == '\t' ||
          *ptr == '\n' ||
          *ptr == '\r')) {
    ptr++;
    count--;
  }
  return limit - count;
}

int JSONSkipWhiteSpaceC (uint8_t const*& ptr, int limit) {
  return JSONSkipWhiteSpaceInline(ptr, limit);
}

extern int (*JSONSkipWhiteSpace)(uint8_t const*&, int);

#if defined(__SSE4_2__) && ! defined(NO_SSE42)

#include <cpuid.h>
#include <x86intrin.h>

static int JSONStringCopySSE42 (uint8_t*& dst, uint8_t const*& src, int limit) {
  alignas(16) static char const ranges[16] = "\x00\x1f\"\"\\\\";
  __m128i const r = _mm_load_si128(reinterpret_cast<__m128i const*>(ranges));
  int count = 0;
  int x = 0;
  while (limit >= 16) {
    __m128i const s = _mm_loadu_si128(reinterpret_cast<__m128i const*>(src));
    x = _mm_cmpestri(r, 6, s, 16,
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
  if (limit == 0) {
    return count;
  }
  __m128i const s = _mm_loadu_si128(reinterpret_cast<__m128i const*>(src));
  x = _mm_cmpestri(r, 6, s, limit,
                   _SIDD_UBYTE_OPS | _SIDD_CMP_RANGES |
                   _SIDD_POSITIVE_POLARITY |
                   _SIDD_LEAST_SIGNIFICANT);
  if (x > limit) {
    x = limit;
  }
  memcpy(dst, src, x);
  dst += x;
  src += x;
  count += x;
  return count;
}

static bool HasSSE42 () {
  unsigned int eax, ebx, ecx, edx;
  if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
    if ((ecx & 0x100000) != 0) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

static int DoInitCopy (uint8_t*& dst, uint8_t const*& src, int limit) {
  if (HasSSE42()) {
    JSONStringCopy = JSONStringCopySSE42;
  }
  else {
    JSONStringCopy = JSONStringCopyC;
  }
  return (*JSONStringCopy)(dst, src, limit);
}

static int JSONSkipWhiteSpaceSSE42 (uint8_t const*& ptr, int limit) {
  alignas(16) static char const white[16] = " \t\n\r";
  __m128i const w = _mm_load_si128(reinterpret_cast<__m128i const*>(white));
  int count = 0;
  int x = 0;
  while (limit >= 16) {
    __m128i const s = _mm_loadu_si128(reinterpret_cast<__m128i const*>(ptr));
    x = _mm_cmpestri(w, 4, s, 16,
                     _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY |
                     _SIDD_NEGATIVE_POLARITY |
                     _SIDD_LEAST_SIGNIFICANT);
    if (x < 16) {
      ptr += x;
      count += x;
      return count;
    }
    ptr += 16;
    limit -= 16;
    count += 16;
  }
  if (limit == 0) {
    return count;
  }
  __m128i const s = _mm_loadu_si128(reinterpret_cast<__m128i const*>(ptr));
  x = _mm_cmpestri(w, 4, s, limit,
                   _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY |
                   _SIDD_NEGATIVE_POLARITY |
                   _SIDD_LEAST_SIGNIFICANT);
  if (x > limit) {
    x = limit;
  }
  ptr += x;
  count += x;
  return count;
}

static int DoInitSkip (uint8_t const*& ptr, int limit) {
  if (HasSSE42()) {
    JSONSkipWhiteSpace = JSONSkipWhiteSpaceSSE42;
  }
  else {
    JSONSkipWhiteSpace = JSONSkipWhiteSpaceC;
  }
  return (*JSONSkipWhiteSpace)(ptr, limit);
}

#else

static int DoInitCopy (uint8_t*& dst, uint8_t const*& src, int limit) {
  JSONStringCopy = JSONStringCopyC;
  return JSONStringCopyC(dst, src, limit);
}

static int DoInitSkip (uint8_t const*& ptr, int limit) {
  JSONSkipWhiteSpace = JSONSkipWhiteSpaceC;
  return JSONSkipWhiteSpace(ptr, limit);
}

#endif

