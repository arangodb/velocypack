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

int JSONStringCopyC (uint8_t*& dst, uint8_t const*& src, int limit);
extern int (*JSONStringCopy)(uint8_t*&, uint8_t const*&, int);

// Now a version which also stops at high bit set bytes:

static inline int JSONStringCopyCheckUtf8Inline (uint8_t*& dst,
                                                 uint8_t const*& src,
                                                 int limit) {
  // Copy up to limit uint8_t from src to dst.
  // Stop at the first control character or backslash or double quote.
  // Also stop at byte with high bit set.
  // Report the number of bytes copied. May copy less bytes, for example
  // for alignment reasons.
  int count = limit;
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

int JSONStringCopyCheckUtf8C (uint8_t*& dst, uint8_t const*& src, int limit);
extern int (*JSONStringCopyCheckUtf8)(uint8_t*&, uint8_t const*&, int);

// White space skipping:

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

int JSONSkipWhiteSpaceC (uint8_t const*& ptr, int limit);
extern int (*JSONSkipWhiteSpace)(uint8_t const*&, int);

