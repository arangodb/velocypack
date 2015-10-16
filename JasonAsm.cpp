#include <iostream>
#include <chrono>

#include "JasonAsm.h"

int (*JSONStringCopy)(uint8_t*&, uint8_t const*&, int) = DoInit;

#if defined(COMPILE_JASONASM_UNITTESTS)

int testPositions[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                        16, 23, 31, 32, 67, 103, 178, 210, 234, 247, 254, 255,
                        -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, 
                        -13, -14, -15, -16, -23, -31, -32, -67, -103, -178,
                        -210, -234, -247, -254, -255 };

int main (int argc, char* argv[]) { 
  if (argc < 4) {
    std::cout << "Usage: JasonAsm SIZE REPEAT CORRECTNESS" << std::endl;
    return 0;
  }

  int size = atoi(argv[1]);
  int repeat = atoi(argv[2]);
  int docorrectness = atoi(argv[3]);
  std::cout << "Size: " << size << std::endl;
  std::cout << "Repeat:" << repeat << std::endl;

  uint8_t* src = new uint8_t[size+17];
  uint8_t* dst = new uint8_t[size+17];
  for (int i = 0; i < size+16; i++) {
    src[i] = 'a' + (i % 26);
  }
  src[size+16] = 0;

  uint8_t const* srcx;
  uint8_t* dstx;
  int copied;

  if (docorrectness > 0) {
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

    decltype(start) now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> totalTime 
        = std::chrono::duration_cast<std::chrono::duration<double>>(now - start);
    std::cout << "Tests took altogether " << totalTime.count() << " seconds." 
              << std::endl;
  }

  std::cout << "\nNow racing for the repeated full string, first target aligned...\n" << std::endl;

  auto start2 = std::chrono::high_resolution_clock::now();
  int akku = 0;
  src[size] = 0;
  for (int j = 0; j < repeat; j++) {
    srcx = src; dstx = dst;
    copied = JSONStringCopy(dstx, srcx, size);
    akku = akku * 13 + copied;
  }
  decltype(start2) now2 = std::chrono::high_resolution_clock::now();

  auto totalTime2 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now2 - start2);

  std::cout << "Race took altogether " << totalTime2.count() << " seconds." 
            << std::endl;
  std::cout << "Time to copy string of length " << size+16
            << " on average is: " << totalTime2.count() / repeat << "."
            << std::endl;
  std::cout << "Megabytes copied per second: "
            << (double) size * (double) repeat / totalTime2.count()
            << std::endl;

  std::cout << "\nNow racing for the repeated full string, now unaligned target...\n" << std::endl;

  start2 = std::chrono::high_resolution_clock::now();
  dst++;
  for (int j = 0; j < repeat; j++) {
    srcx = src; dstx = dst;
    copied = JSONStringCopy(dstx, srcx, size);
    akku = akku * 13 + copied;
  }
  now2 = std::chrono::high_resolution_clock::now();

  totalTime2 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now2 - start2);

  std::cout << "Race took altogether " << totalTime2.count() << " seconds." 
            << std::endl;
  std::cout << "Time to copy string of length " << size+16
            << " on average is: " << totalTime2.count() / repeat << "."
            << std::endl;
  std::cout << "Megabytes copied per second: "
            << (double) size * (double) repeat / totalTime2.count()
            << std::endl;

  std::cout << "\nNow comparing with strcpy...\n" << std::endl;

  start2 = std::chrono::high_resolution_clock::now();
  dst--;
  for (int j = 0; j < repeat; j++) {
    srcx = src; dstx = dst;
    strcpy((char*) dstx, (char*) srcx);
  }
  now2 = std::chrono::high_resolution_clock::now();

  totalTime2 
      = std::chrono::duration_cast<std::chrono::duration<double>>(now2 - start2);

  std::cout << "Race took altogether " << totalTime2.count() << " seconds." 
            << std::endl;
  std::cout << "Time to copy string of length " << size+16
            << " on average is: " << totalTime2.count() / repeat << "."
            << std::endl;
  std::cout << "Megabytes copied per second: "
            << (double) size * (double) repeat / totalTime2.count()
            << std::endl;

  //std::cout << "\n\nAkku (just ignore, this is to beat the compiler optimization): "
  //          << akku << std::endl;
  return 0;
}

#endif
