#include <cstdint>
#include <cstring>

static inline size_t JSONStringCopyInline (uint8_t*& dst, uint8_t const*& src,
                                           size_t limit) {
  // Copy up to limit uint8_t from src to dst.
  // Stop at the first control character or backslash or double quote.
  // Report the number of bytes copied. May copy less bytes, for example
  // for alignment reasons.
  size_t count = limit;
  while (count > 0 && 
         *src >= 32 && 
         *src != '\\' && 
         *src != '"') {
    *dst++ = *src++;
    count--;
  }
  return limit - count;
}

size_t JSONStringCopyC (uint8_t*& dst, uint8_t const*& src, size_t limit);
extern size_t (*JSONStringCopy)(uint8_t*&, uint8_t const*&, size_t);

// Now a version which also stops at high bit set bytes:

static inline size_t JSONStringCopyCheckUtf8Inline (uint8_t*& dst,
                                                    uint8_t const*& src,
                                                    size_t limit) {
  // Copy up to limit uint8_t from src to dst.
  // Stop at the first control character or backslash or double quote.
  // Also stop at byte with high bit set.
  // Report the number of bytes copied. May copy less bytes, for example
  // for alignment reasons.
  size_t count = limit;
  while (count > 0 && 
         *src >= 32 && 
         *src != '\\' && 
         *src != '"' &&
         *src < 0x80) {
    *dst++ = *src++;
    count--;
  }
  return limit - count;
}

size_t JSONStringCopyCheckUtf8C (uint8_t*& dst, uint8_t const*& src,
                                 size_t limit);
extern size_t (*JSONStringCopyCheckUtf8)(uint8_t*&, uint8_t const*&, size_t);

// White space skipping:

static inline size_t JSONSkipWhiteSpaceInline (uint8_t const*& ptr,
                                               size_t limit) {
  // Skip up to limit uint8_t from ptr as long as they are whitespace.
  // Advance ptr and return the number of skipped bytes.
  size_t count = limit;
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

size_t JSONSkipWhiteSpaceC (uint8_t const*& ptr, size_t limit);
extern size_t (*JSONSkipWhiteSpace)(uint8_t const*&, size_t);

