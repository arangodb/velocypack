#include <iostream>
#include <chrono>

#include "JasonAsm.h"

int JSONStringCopyC (uint8_t*& dst, uint8_t const*& src, int limit) {
  return JSONStringCopyInline(dst, src, limit);
}

int JSONStringCopyCheckUtf8C (uint8_t*& dst, uint8_t const*& src, int limit) {
  return JSONStringCopyCheckUtf8Inline(dst, src, limit);
}

int JSONSkipWhiteSpaceC (uint8_t const*& ptr, int limit) {
  return JSONSkipWhiteSpaceInline(ptr, limit);
}

#if defined(__SSE4_2__) && ! defined(NO_SSE42)

#include <cpuid.h>
#include <x86intrin.h>

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

static int DoInitCopy (uint8_t*& dst, uint8_t const*& src, int limit) {
  if (HasSSE42()) {
    JSONStringCopy = JSONStringCopySSE42;
  }
  else {
    JSONStringCopy = JSONStringCopyC;
  }
  return (*JSONStringCopy)(dst, src, limit);
}

static int JSONStringCopyCheckUtf8SSE42 (uint8_t*& dst,
                                         uint8_t const*& src,
                                         int limit) {
  alignas(16) static unsigned char const ranges[16] = "\x00\x1f\x80\xff\"\"\\\\";
  __m128i const r = _mm_load_si128(reinterpret_cast<__m128i const*>(ranges));
  int count = 0;
  int x = 0;
  while (limit >= 16) {
    __m128i const s = _mm_loadu_si128(reinterpret_cast<__m128i const*>(src));
    x = _mm_cmpestri(r, 8, s, 16,
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
  x = _mm_cmpestri(r, 8, s, limit,
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

static int DoInitCopyCheckUtf8 (uint8_t*& dst, uint8_t const*& src, int limit) {
  if (HasSSE42()) {
    JSONStringCopyCheckUtf8 = JSONStringCopyCheckUtf8SSE42;
  }
  else {
    JSONStringCopyCheckUtf8 = JSONStringCopyCheckUtf8C;
  }
  return (*JSONStringCopyCheckUtf8)(dst, src, limit);
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

static int DoInitCopyCheckUtf8 (uint8_t*& dst, uint8_t const*& src, int limit) {
  JSONStringCopyCheckUtf8 = JSONStringCopyCheckUtf8C;
  return JSONStringCopyCheckUtf8C(dst, src, limit);
}

static int DoInitSkip (uint8_t const*& ptr, int limit) {
  JSONSkipWhiteSpace = JSONSkipWhiteSpaceC;
  return JSONSkipWhiteSpace(ptr, limit);
}

#endif


int (*JSONStringCopy)(uint8_t*&, uint8_t const*&, int) = DoInitCopy;
int (*JSONStringCopyCheckUtf8)(uint8_t*&, uint8_t const*&, int) 
    = DoInitCopyCheckUtf8;
int (*JSONSkipWhiteSpace)(uint8_t const*&, int) = DoInitSkip;

#if defined(COMPILE_JASONASM_UNITTESTS)

int testPositions[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                        16, 23, 31, 32, 67, 103, 178, 210, 234, 247, 254, 255,
                        -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, 
                        -13, -14, -15, -16, -23, -31, -32, -67, -103, -178,
                        -210, -234, -247, -254, -255 };

void TestStringCopyCorrectness (uint8_t* src, uint8_t* dst, int size) {
  uint8_t const* srcx;
  uint8_t* dstx;
  int copied;

  std::cout << "Performing correctness tests..." << std::endl;

  auto start = std::chrono::high_resolution_clock::now();

  for (int salign = 0; salign < 16; salign++) {
    src += salign;
    for (int dalign = 0; dalign < 16; dalign++) {
      dst += dalign;
      for (int i = 0;
           i < static_cast<int>(sizeof(testPositions) / sizeof(int)); i++) {
        uint8_t merk;
        int pos = testPositions[i];
        if (pos < 0) {
          pos = size + pos;
        }
        if (pos < 0 || pos >= size) {
          continue;
        }

        // Test a quote character:
        merk = src[pos]; src[pos] = '"';
        srcx = src; dstx = dst;
        copied = JSONStringCopy(dstx, srcx, size);
        if (copied != pos || memcmp(dst, src, copied) != 0) {
          std::cout << "Error: " << salign << " " << dalign << " "
                    << i << " " << pos << " " << copied << std::endl;
        }
        src[pos] = merk;

        // Test a backslash character:
        src[pos] = '\\';
        srcx = src; dstx = dst;
        copied = JSONStringCopy(dstx, srcx, size);
        if (copied != pos || memcmp(dst, src, copied) != 0) {
          std::cout << "Error: " << salign << " " << dalign << " "
                    << i << " " << pos << " " << copied << std::endl;
        }
        src[pos] = merk;

        // Test a 0 character:
        src[pos] = 0;
        srcx = src; dstx = dst;
        copied = JSONStringCopy(dstx, srcx, size);
        if (copied != pos || memcmp(dst, src, copied) != 0) {
          std::cout << "Error: " << salign << " " << dalign << " "
                    << i << " " << pos << " " << copied << std::endl;
        }
        src[pos] = merk;

        // Test a control character:
        src[pos] = 31;
        srcx = src; dstx = dst;
        copied = JSONStringCopy(dstx, srcx, size);
        if (copied != pos || memcmp(dst, src, copied) != 0) {
          std::cout << "Error: " << salign << " " << dalign << " "
                    << i << " " << pos << " " << copied << std::endl;
        }
        src[pos] = merk;
      }
      dst -= dalign;
    }
    src -= salign;
  }

  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> totalTime 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now - start);
  std::cout << "Tests took altogether " << totalTime.count() << " seconds." 
            << std::endl;
}

void TestStringCopyCorrectnessCheckUtf8 (uint8_t* src, uint8_t* dst, int size) {
  uint8_t const* srcx;
  uint8_t* dstx;
  int copied;

  std::cout << "Performing correctness tests (check UTF8)..." << std::endl;

  auto start = std::chrono::high_resolution_clock::now();

  for (int salign = 0; salign < 16; salign++) {
    src += salign;
    for (int dalign = 0; dalign < 16; dalign++) {
      dst += dalign;
      for (int i = 0;
           i < static_cast<int>(sizeof(testPositions) / sizeof(int)); i++) {
        uint8_t merk;
        int pos = testPositions[i];
        if (pos < 0) {
          pos = size + pos;
        }
        if (pos < 0 || pos >= size) {
          continue;
        }

        // Test a quote character:
        merk = src[pos]; src[pos] = '"';
        srcx = src; dstx = dst;
        copied = JSONStringCopyCheckUtf8(dstx, srcx, size);
        if (copied != pos || memcmp(dst, src, copied) != 0) {
          std::cout << "Error: " << salign << " " << dalign << " "
                    << i << " " << pos << " " << copied << std::endl;
        }
        src[pos] = merk;

        // Test a backslash character:
        src[pos] = '\\';
        srcx = src; dstx = dst;
        copied = JSONStringCopyCheckUtf8(dstx, srcx, size);
        if (copied != pos || memcmp(dst, src, copied) != 0) {
          std::cout << "Error: " << salign << " " << dalign << " "
                    << i << " " << pos << " " << copied << std::endl;
        }
        src[pos] = merk;

        // Test a 0 character:
        src[pos] = 0;
        srcx = src; dstx = dst;
        copied = JSONStringCopyCheckUtf8(dstx, srcx, size);
        if (copied != pos || memcmp(dst, src, copied) != 0) {
          std::cout << "Error: " << salign << " " << dalign << " "
                    << i << " " << pos << " " << copied << std::endl;
        }
        src[pos] = merk;

        // Test a control character:
        src[pos] = 31;
        srcx = src; dstx = dst;
        copied = JSONStringCopyCheckUtf8(dstx, srcx, size);
        if (copied != pos || memcmp(dst, src, copied) != 0) {
          std::cout << "Error: " << salign << " " << dalign << " "
                    << i << " " << pos << " " << copied << std::endl;
        }
        src[pos] = merk;

        // Test a high bitcharacter:
        src[pos] = 0x80;
        srcx = src; dstx = dst;
        copied = JSONStringCopyCheckUtf8(dstx, srcx, size);
        if (copied != pos || memcmp(dst, src, copied) != 0) {
          std::cout << "Error: " << salign << " " << dalign << " "
                    << i << " " << pos << " " << copied << std::endl;
        }
        src[pos] = merk;
      }
      dst -= dalign;
    }
    src -= salign;
  }

  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> totalTime 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now - start);
  std::cout << "Tests took altogether " << totalTime.count() << " seconds." 
            << std::endl;
}

void TestSkipWhiteSpaceCorrectness (uint8_t* src, int size) {
  uint8_t const* srcx;
  int copied;

  std::cout << "Performing correctness tests for whitespace skipping..." 
            << std::endl;

  auto start = std::chrono::high_resolution_clock::now();

  for (int salign = 0; salign < 16; salign++) {
    src += salign;
    for (int i = 0;
         i < static_cast<int>(sizeof(testPositions) / sizeof(int)); i++) {
      uint8_t merk;
      int pos = testPositions[i];
      if (pos < 0) {
        pos = size + pos;
      }
      if (pos < 0 || pos >= size) {
        continue;
      }

      // Test a non-whitespace character:
      merk = src[pos]; src[pos] = 'x';
      srcx = src;
      copied = JSONSkipWhiteSpace(srcx, size);
      if (copied != pos) {
        std::cout << "Error: " << salign << " "
                  << i << " " << pos << " " << copied << std::endl;
      }
      src[pos] = merk;
    }
    src -= salign;
  }

  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> totalTime 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now-start);
  std::cout << "Whitespace tests took altogether " << totalTime.count() 
            << " seconds." << std::endl;
}

void RaceStringCopy (uint8_t* dst, uint8_t* src, int size, int repeat, int&akku) {
  uint8_t const* srcx;
  uint8_t* dstx;
  int copied;

  std::cout << "\nNow racing for the repeated full string, "
            << "first target aligned...\n" << std::endl;

  src[size] = 0;
  auto start = std::chrono::high_resolution_clock::now();
  for (int j = 0; j < repeat; j++) {
    srcx = src; dstx = dst;
    copied = JSONStringCopy(dstx, srcx, size);
    akku = akku * 13 + copied;
  }
  auto now = std::chrono::high_resolution_clock::now();
  src[size] = 'a' + (size % 26);

  auto totalTime 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now - start);

  std::cout << "Race took altogether " << totalTime.count() << " seconds." 
            << std::endl;
  std::cout << "Time to copy string of length " << size
            << " on average is: " << totalTime.count() / repeat << "."
            << std::endl;
  std::cout << "Bytes copied per second: "
            << (double) size * (double) repeat / totalTime.count()
            << std::endl;

  std::cout << "\nNow racing for the repeated full string, "
            << "now unaligned target...\n" << std::endl;

  dst++;
  start = std::chrono::high_resolution_clock::now();
  for (int j = 0; j < repeat; j++) {
    srcx = src; dstx = dst;
    copied = JSONStringCopy(dstx, srcx, size);
    akku = akku * 13 + copied;
  }
  now = std::chrono::high_resolution_clock::now();
  dst--;

  totalTime 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now - start);

  std::cout << "Race took altogether " << totalTime.count() << " seconds." 
            << std::endl;
  std::cout << "Time to copy string of length " << size
            << " on average is: " << totalTime.count() / repeat << "."
            << std::endl;
  std::cout << "Bytes copied per second: "
            << (double) size * (double) repeat / totalTime.count()
            << std::endl;
}

void RaceStringCopyCheckUtf8 (uint8_t* dst, uint8_t* src,
                              int size, int repeat, int& akku) {
  uint8_t const* srcx;
  uint8_t* dstx;
  int copied;

  std::cout << "\nNow racing for the repeated (check UTF8) full string, "
            << "first target aligned...\n" << std::endl;

  src[size] = 0;
  auto start = std::chrono::high_resolution_clock::now();
  for (int j = 0; j < repeat; j++) {
    srcx = src; dstx = dst;
    copied = JSONStringCopy(dstx, srcx, size);
    akku = akku * 13 + copied;
  }
  auto now = std::chrono::high_resolution_clock::now();
  src[size] = 'a' + (size % 26);

  auto totalTime 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now - start);

  std::cout << "Race took altogether " << totalTime.count() << " seconds." 
            << std::endl;
  std::cout << "Time to copy string of length " << size
            << " on average is: " << totalTime.count() / repeat << "."
            << std::endl;
  std::cout << "Bytes copied per second: "
            << (double) size * (double) repeat / totalTime.count()
            << std::endl;

  std::cout << "\nNow racing for the repeated full string, now unaligned target...\n" << std::endl;

  dst++;
  start = std::chrono::high_resolution_clock::now();
  for (int j = 0; j < repeat; j++) {
    srcx = src; dstx = dst;
    copied = JSONStringCopy(dstx, srcx, size);
    akku = akku * 13 + copied;
  }
  now = std::chrono::high_resolution_clock::now();
  dst--;

  totalTime 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now - start);

  std::cout << "Race took altogether " << totalTime.count() << " seconds." 
            << std::endl;
  std::cout << "Time to copy string of length " << size
            << " on average is: " << totalTime.count() / repeat << "."
            << std::endl;
  std::cout << "Bytes copied per second: "
            << (double) size * (double) repeat / totalTime.count()
            << std::endl;

  std::cout << "\nNow comparing with strcpy...\n" << std::endl;

  start = std::chrono::high_resolution_clock::now();
  for (int j = 0; j < repeat; j++) {
    srcx = src; dstx = dst;
    strcpy((char*) dstx, (char*) srcx);
  }
  now = std::chrono::high_resolution_clock::now();

  totalTime 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now - start);

  std::cout << "Race took altogether " << totalTime.count() << " seconds." 
            << std::endl;
  std::cout << "Time to copy string of length " << size
            << " on average is: " << totalTime.count() / repeat << "."
            << std::endl;
  std::cout << "Bytes copied per second: "
            << (double) size * (double) repeat / totalTime.count()
            << std::endl;
}

void RaceSkipWhiteSpace (uint8_t* src, int size, int repeat, int& akku) {
  uint8_t const* srcx;
  int copied;

  std::cout << "\nNow racing for the repeated full string...\n" << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  akku = 0;
  src[size] = 0;
  for (int j = 0; j < repeat; j++) {
    srcx = src;
    copied = JSONSkipWhiteSpace(srcx, size);
    akku = akku * 13 + copied;
  }
  auto now = std::chrono::high_resolution_clock::now();

  auto totalTime 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now-start);

  std::cout << "Race took altogether " << totalTime.count() << " seconds." 
            << std::endl;
  std::cout << "Time to skip white string of length " << size
            << " on average is: " << totalTime.count() / repeat << "."
            << std::endl;
  std::cout << "Bytes skipped per second: "
            << (double) size * (double) repeat / totalTime.count()
            << std::endl;

  std::cout << "\nNow comparing with strlen...\n" << std::endl;

  start = std::chrono::high_resolution_clock::now();
  for (int j = 0; j < repeat; j++) {
    srcx = src;
    copied = strlen((char*) srcx);
    // Fake activity for the compiler:
    src[0] = (j & 0xf) + 1;
    akku = akku * 13 + copied;
  }
  now = std::chrono::high_resolution_clock::now();

  totalTime 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now - start);

  std::cout << "Race took altogether " << totalTime.count() << " seconds." 
            << std::endl;
  std::cout << "Time to strlen string of length " << size
            << " on average is: " << totalTime.count() / repeat << "."
            << std::endl;
  std::cout << "Bytes scanned per second: "
            << (double) size * (double) repeat / totalTime.count()
            << std::endl;
}

int main (int argc, char* argv[]) { 
  if (argc < 4) {
    std::cout << "Usage: JasonAsm SIZE REPEAT CORRECTNESS" << std::endl;
    return 0;
  }

  int size = atoi(argv[1]);
  int repeat = atoi(argv[2]);
  int docorrectness = atoi(argv[3]);
  int akku = 0;
  std::cout << "Size: " << size << std::endl;
  std::cout << "Repeat:" << repeat << std::endl;

  uint8_t* src = new uint8_t[size+17];
  uint8_t* dst = new uint8_t[size+17];
  std::cout << "Src pointer: " << (void*) src << std::endl;
  std::cout << "Dst pointer: " << (void*) dst << std::endl;
  for (int i = 0; i < size+16; i++) {
    src[i] = 'a' + (i % 26);
  }
  src[size+16] = 0;

  if (docorrectness > 0) {
    TestStringCopyCorrectness(src, dst, size); 
  }

  RaceStringCopy(dst, src, size, repeat, akku);
  
  if (docorrectness > 0) {
    TestStringCopyCorrectnessCheckUtf8(src, dst, size); 
  }

  RaceStringCopyCheckUtf8(dst, src, size, repeat, akku);

  std::cout << "\n\n\nNOW WHITESPACE SKIPPING\n" << std::endl;

  // Now do the whitespace skipping tests/measurements:
  static char const whitetab[17] = "       \t   \n   \r";
  for (int i = 0; i < size+16; i++) {
    src[i] = whitetab[i % 16];
  }
  src[size+16] = 0;

  if (docorrectness > 0) {
    TestSkipWhiteSpaceCorrectness(src, size);
  }

  RaceSkipWhiteSpace(src, size, repeat, akku);

  std::cout << "\n\n\nAkku (please ignore):" << akku << std::endl;
  std::cout << "\n\n\nGuck (please ignore): " << dst[100] << std::endl;

  delete[] src;
  delete[] dst;
  return 0;
}

#endif
