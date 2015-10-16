static int JSONStringCopySSE42_0 (uint8_t*& dst, uint8_t const*& src, int limit) {
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

static int JSONStringCopySSE42_3 (uint8_t*& dst, uint8_t const*& src, int limit) {
  alignas(16) static char const ranges[16] = "\x00\x1f\"\"\\\\";
  __m128i const r = _mm_load_si128(reinterpret_cast<__m128i const*>(ranges));
  int merklimit = limit;
  int x = 0;
  while (limit >= 16) {
    __m128i const s = _mm_loadu_si128(reinterpret_cast<__m128i const*>(src));
    x = _mm_cmpestri(r,6, s, 16,
                     _SIDD_UBYTE_OPS | _SIDD_CMP_RANGES |
                     _SIDD_POSITIVE_POLARITY |
                     _SIDD_LEAST_SIGNIFICANT);
    if (x < 16) {
      memcpy(dst, src, x);
      dst += x;
      src += x;
      limit -= x;
      return merklimit - limit;
    }
    _mm_storeu_si128(reinterpret_cast<__m128i*>(dst), s);
    src += 16;
    dst += 16;
    limit -= 16;
  }
  if (limit == 0) {
    return merklimit;
  }
  __m128i const s = _mm_loadu_si128(reinterpret_cast<__m128i const*>(src));
  x = _mm_cmpestri(r,6, s, limit,
                   _SIDD_UBYTE_OPS | _SIDD_CMP_RANGES |
                   _SIDD_POSITIVE_POLARITY |
                   _SIDD_LEAST_SIGNIFICANT);
  if (x > limit) {
    x = limit;
  }
  memcpy(dst, src, x);
  dst += x;
  src += x;
  limit -= x;
  return merklimit - limit;
}

