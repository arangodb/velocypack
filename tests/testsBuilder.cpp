////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up VPack documents.
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
/// @author Jan Steemann
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <fstream>
#include <string>

#include "tests-common.h"

TEST(BuilderTest, Null) {
  Builder b;
  b.add(Value());
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t const correctResult[] = { 0x18 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, False) {
  Builder b;
  b.add(Value(false));
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t const correctResult[] = { 0x19 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, True) {
  Builder b;
  b.add(Value(true));
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t const correctResult[] = { 0x1a };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Double) {
  static double value = 123.456;
  Builder b;
  b.add(Value(value));
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[9] = { 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  ASSERT_EQ(8ULL, sizeof(double));
  dumpDouble(value, correctResult + 1);

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, String) {
  Builder b;
  b.add(Value("abcdefghijklmnopqrstuvwxyz"));
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x5a, 
        0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
        0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
        0x77, 0x78, 0x79, 0x7a };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ArrayEmpty) {
  Builder b;
  b.add(Value(ValueType::Array));
  b.close();
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[] = { 0x01 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ArraySingleEntry) {
  Builder b;
  b.add(Value(ValueType::Array));
  b.add(Value(uint64_t(1)));
  b.close();
  uint8_t* result = b.start();
  ASSERT_EQ(0x02U, *result);
  ValueLength len = b.size();

  static uint8_t correctResult[] = { 0x02, 0x03, 0x31 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ArraySingleEntryLong) {
  std::string const value("ngdddddljjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjsdddffffffffffffmmmmmmmmmmmmmmmsfdlllllllllllllllllllllllllllllllllllllllllllllllllrjjjjjjsddddddddddddddddddhhhhhhkkkkkkkksssssssssssssssssssssssssssssssssdddddddddddddddddkkkkkkkkkkkkksddddddddddddssssssssssfvvvvvvvvvvvvvvvvvvvvvvvvvvvfvgfff");
  Builder b;
  b.add(Value(ValueType::Array));
  b.add(Value(value));
  b.close();
  uint8_t* result = b.start();
  ASSERT_EQ(0x03U, *result);
  ValueLength len = b.size(); 

  static uint8_t correctResult[] = {
    0x03, 0x2c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0x1a, 0x01, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x6e, 0x67, 0x64, 0x64, 0x64, 0x64, 0x64, 0x6c, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 
    0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 
    0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x73, 0x64, 0x64, 0x64, 0x66, 0x66, 0x66, 
    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 
    0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x73, 0x66, 0x64, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 
    0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 
    0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 
    0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x72, 0x6a, 0x6a, 0x6a, 
    0x6a, 0x6a, 0x6a, 0x73, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 
    0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x6b, 0x6b, 0x6b, 0x6b, 
    0x6b, 0x6b, 0x6b, 0x6b, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 
    0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 
    0x73, 0x73, 0x73, 0x73, 0x73, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 
    0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 
    0x6b, 0x6b, 0x6b, 0x73, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 
    0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x66, 0x76, 0x76, 0x76, 0x76, 0x76, 
    0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 
    0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x66, 0x76, 0x67, 0x66, 0x66, 0x66     
  };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ArraySameSizeEntries) {
  Builder b;
  b.add(Value(ValueType::Array));
  b.add(Value(uint64_t(1)));
  b.add(Value(uint64_t(2)));
  b.add(Value(uint64_t(3)));
  b.close();
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[] = { 0x02, 0x05, 0x31, 0x32, 0x33 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Array4) {
  double value = 2.3;
  Builder b;
  b.add(Value(ValueType::Array));
  b.add(Value(uint64_t(1200)));
  b.add(Value(value));
  b.add(Value("abc"));
  b.add(Value(true));
  b.close();

  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x06, 0x18, 0x04,
        0x29, 0xb0, 0x04,   // uint(1200) = 0x4b0
        0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // double(2.3)
        0x43, 0x61, 0x62, 0x63,
        0x1a,
        0x03, 0x06, 0x0f, 0x13};
  dumpDouble(value, correctResult + 7);

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ObjectEmpty) {
  Builder b;
  b.add(Value(ValueType::Object));
  b.close();
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[] = { 0x0a };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ObjectSorted) {
  double value = 2.3;
  Builder b;
  b.options.sortAttributeNames = true;
  b.add(Value(ValueType::Object));
  b.add("d", Value(uint64_t(1200)));
  b.add("c", Value(value));
  b.add("b", Value("abc"));
  b.add("a", Value(true));
  b.close();

  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x0b, 0x20, 0x04,
        0x41, 0x64, 0x29, 0xb0, 0x04,        // "d": uint(1200) = 0x4b0
        0x41, 0x63, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   
                                             // "c": double(2.3)
        0x41, 0x62, 0x43, 0x61, 0x62, 0x63,  // "b": "abc"
        0x41, 0x61, 0x1a,                    // "a": true
        0x19, 0x13, 0x08, 0x03
      };
  dumpDouble(value, correctResult + 11);

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ObjectUnsorted) {
  double value = 2.3;
  Builder b;
  b.options.sortAttributeNames = false;
  b.add(Value(ValueType::Object));
  b.add("d", Value(uint64_t(1200)));
  b.add("c", Value(value));
  b.add("b", Value("abc"));
  b.add("a", Value(true));
  b.close();

  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x0f, 0x20, 0x04,
        0x41, 0x64, 0x29, 0xb0, 0x04,        // "d": uint(1200) = 0x4b0
        0x41, 0x63, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   
                                             // "c": double(2.3)
        0x41, 0x62, 0x43, 0x61, 0x62, 0x63,  // "b": "abc"
        0x41, 0x61, 0x1a,                    // "a": true
        0x03, 0x08, 0x13, 0x19
      };
  dumpDouble(value, correctResult + 11);

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Object4) {
  double value = 2.3;
  Builder b;
  b.add(Value(ValueType::Object));
  b.add("a", Value(uint64_t(1200)));
  b.add("b", Value(value));
  b.add("c", Value("abc"));
  b.add("d", Value(true));
  b.close();

  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x0b, 0x20, 0x04,
        0x41, 0x61, 0x29, 0xb0, 0x04,        // "a": uint(1200) = 0x4b0
        0x41, 0x62, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   
                                             // "b": double(2.3)
        0x41, 0x63, 0x43, 0x61, 0x62, 0x63,  // "c": "abc"
        0x41, 0x64, 0x1a,                    // "d": true
        0x03, 0x08, 0x13, 0x19
      };
  dumpDouble(value, correctResult + 11);

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, External) {
  uint8_t externalStuff[] = { 0x01 };
  Builder b;
  b.add(Value(const_cast<void const*>(static_cast<void*>(externalStuff)), 
              ValueType::External));
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[1 + sizeof(char*)] = { 0x00 };
  correctResult[0] = 0x1d;
  uint8_t* p = externalStuff;
  memcpy(correctResult + 1, &p, sizeof(uint8_t*));

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ExternalUTCDate) {
  int64_t const v = -24549959465;
  Builder bExternal;
  bExternal.add(Value(v, ValueType::UTCDate));

  Builder b;
  b.add(Value(const_cast<void const*>(static_cast<void*>(bExternal.start()))));
  
  Slice s(b.start());
  ASSERT_EQ(ValueType::External, s.type());
#ifdef VELOCYPACK_64BIT
  ASSERT_EQ(9ULL, s.byteSize());
#else
  ASSERT_EQ(5ULL, s.byteSize());
#endif
  Slice sExternal(s.getExternal());
  ASSERT_EQ(9ULL, sExternal.byteSize());
  ASSERT_EQ(ValueType::UTCDate, sExternal.type());
  ASSERT_EQ(v, sExternal.getUTCDate());
}

TEST(BuilderTest, ExternalDouble) {
  double const v = -134.494401;
  Builder bExternal;
  bExternal.add(Value(v));

  Builder b;
  b.add(Value(const_cast<void const*>(static_cast<void*>(bExternal.start()))));
  
  Slice s(b.start());
  ASSERT_EQ(ValueType::External, s.type());
#ifdef VELOCYPACK_64BIT
  ASSERT_EQ(9ULL, s.byteSize());
#else
  ASSERT_EQ(5ULL, s.byteSize());
#endif 

  Slice sExternal(s.getExternal());
  ASSERT_EQ(9ULL, sExternal.byteSize());
  ASSERT_EQ(ValueType::Double, sExternal.type());
  ASSERT_DOUBLE_EQ(v, sExternal.getDouble());
}

TEST(BuilderTest, ExternalBinary) {
  char const* p = "the quick brown FOX jumped over the lazy dog";
  Builder bExternal;
  bExternal.add(Value(std::string(p), ValueType::Binary));

  Builder b;
  b.add(Value(const_cast<void const*>(static_cast<void*>(bExternal.start()))));
  
  Slice s(b.start());
  ASSERT_EQ(ValueType::External, s.type());
#ifdef VELOCYPACK_64BIT
  ASSERT_EQ(9ULL, s.byteSize());
#else
  ASSERT_EQ(5ULL, s.byteSize());
#endif 
 
  Slice sExternal(s.getExternal());
  ASSERT_EQ(2 + strlen(p), sExternal.byteSize());
  ASSERT_EQ(ValueType::Binary, sExternal.type());
  ValueLength len;
  uint8_t const* str = sExternal.getBinary(len);
  ASSERT_EQ(strlen(p), len);
  ASSERT_EQ(0, memcmp(str, p, len));
}

TEST(BuilderTest, ExternalString) {
  char const* p = "the quick brown FOX jumped over the lazy dog";
  Builder bExternal;
  bExternal.add(Value(std::string(p)));

  Builder b;
  b.add(Value(const_cast<void const*>(static_cast<void*>(bExternal.start()))));
  
  Slice s(b.start());
  ASSERT_EQ(ValueType::External, s.type());
#ifdef VELOCYPACK_64BIT
  ASSERT_EQ(9ULL, s.byteSize());
#else
  ASSERT_EQ(5ULL, s.byteSize());
#endif 
 
  Slice sExternal(s.getExternal());
  ASSERT_EQ(1 + strlen(p), sExternal.byteSize());
  ASSERT_EQ(ValueType::String, sExternal.type());
  ValueLength len;
  char const* str = sExternal.getString(len);
  ASSERT_EQ(strlen(p), len);
  ASSERT_EQ(0, strncmp(str, p, len));
}

TEST(BuilderTest, ExternalExternal) {
  char const* p = "the quick brown FOX jumped over the lazy dog";
  Builder bExternal;
  bExternal.add(Value(std::string(p)));

  Builder bExExternal;
  bExExternal.add(Value(const_cast<void const*>(static_cast<void*>(bExternal.start()))));
  bExExternal.add(Value(std::string(p)));

  Builder b;
  b.add(Value(const_cast<void const*>(static_cast<void*>(bExExternal.start()))));
  
  Slice s(b.start());
  ASSERT_EQ(ValueType::External, s.type());
#ifdef VELOCYPACK_64BIT
  ASSERT_EQ(9ULL, s.byteSize());
#else
  ASSERT_EQ(5ULL, s.byteSize());
#endif

  Slice sExternal(s.getExternal());
  ASSERT_EQ(ValueType::External, sExternal.type());
#ifdef VELOCYPACK_64BIT
  ASSERT_EQ(9ULL, sExternal.byteSize());
#else
  ASSERT_EQ(5ULL, sExternal.byteSize());
#endif 

  Slice sExExternal(sExternal.getExternal());
  ASSERT_EQ(1 + strlen(p), sExExternal.byteSize());
  ASSERT_EQ(ValueType::String, sExExternal.type());
  ValueLength len;
  char const* str = sExExternal.getString(len);
  ASSERT_EQ(strlen(p), len);
  ASSERT_EQ(0, strncmp(str, p, len));
}

TEST(BuilderTest, UInt) {
  uint64_t value = 0x12345678abcdef;
  Builder b;
  b.add(Value(value));
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[]
    = { 0x2e, 0xef, 0xcd, 0xab, 0x78, 0x56, 0x34, 0x12 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, IntPos) {
  int64_t value = 0x12345678abcdef;
  Builder b;
  b.add(Value(value));
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[]
    = { 0x26, 0xef, 0xcd, 0xab, 0x78, 0x56, 0x34, 0x12 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, IntNeg) {
  int64_t value = -0x12345678abcdef;
  Builder b;
  b.add(Value(value));
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[]
    = { 0x26, 0x11, 0x32, 0x54, 0x87, 0xa9, 0xcb, 0xed };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Int1Limits) {
  int64_t values[] = {-0x80LL, 0x7fLL, -0x81LL, 0x80LL,
                      -0x8000LL, 0x7fffLL, -0x8001LL, 0x8000LL,
                      -0x800000LL, 0x7fffffLL, -0x800001LL, 0x800000LL,
                      -0x80000000LL, 0x7fffffffLL, -0x80000001LL, 0x80000000LL,
                      -0x8000000000LL, 0x7fffffffffLL,
                      -0x8000000001LL, 0x8000000000LL,
                      -0x800000000000LL, 0x7fffffffffffLL, 
                      -0x800000000001LL, 0x800000000000LL,
                      -0x80000000000000LL, 0x7fffffffffffffLL, 
                      -0x80000000000001LL, 0x80000000000000LL,
                      arangodb::velocypack::ToInt64(0x8000000000000000ULL),
                      0x7fffffffffffffffLL};
  for (size_t i = 0; i < sizeof(values) / sizeof(int64_t); i++) {
    int64_t v = values[i];
    Builder b;
    b.add(Value(v));
    uint8_t* result = b.start();
    Slice s(result);
    ASSERT_TRUE(s.isInt());
    ASSERT_EQ(v, s.getInt());
  }
}

TEST(BuilderTest, StringChar) {
  char const* value = "der fuxx ging in den wald und aß pilze";
  size_t const valueLen = strlen(value);
  Builder b;
  b.add(Value(value));

  Slice slice = Slice(b.start());
  ASSERT_TRUE(slice.isString());
 
  ValueLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(valueLen, len);
  ASSERT_EQ(0, strncmp(s, value, valueLen));

  std::string c = slice.copyString();
  ASSERT_EQ(valueLen, c.size());
  ASSERT_EQ(0, strncmp(value, c.c_str(), valueLen));
}

TEST(BuilderTest, StringString) {
  std::string const value("der fuxx ging in den wald und aß pilze");
  Builder b;
  b.add(Value(value));

  Slice slice = Slice(b.start());
  ASSERT_TRUE(slice.isString());
 
  ValueLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(value.size(), len);
  ASSERT_EQ(0, strncmp(s, value.c_str(), value.size()));

  std::string c = slice.copyString();
  ASSERT_EQ(value.size(), c.size());
  ASSERT_EQ(value, c);
}

TEST(BuilderTest, Binary) {
  uint8_t binaryStuff[] = { 0x02, 0x03, 0x05, 0x08, 0x0d };

  Builder b;
  b.add(ValuePair(binaryStuff, sizeof(binaryStuff)));
  uint8_t* result = b.start();
  ValueLength len = b.size();

  static uint8_t correctResult[] = { 0xc0, 0x05, 0x02, 0x03, 0x05, 0x08, 0x0d };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, UTCDate) {
  int64_t const value = 12345678;
  Builder b;
  b.add(Value(value, ValueType::UTCDate));

  Slice s(b.start());
  ASSERT_EQ(0x1cU, s.head());
  ASSERT_TRUE(s.isUTCDate());
  ASSERT_EQ(9UL, s.byteSize());
  ASSERT_EQ(value, s.getUTCDate());
}

TEST(BuilderTest, UTCDateZero) {
  int64_t const value = 0;
  Builder b;
  b.add(Value(value, ValueType::UTCDate));

  Slice s(b.start());
  ASSERT_EQ(0x1cU, s.head());
  ASSERT_TRUE(s.isUTCDate());
  ASSERT_EQ(9UL, s.byteSize());
  ASSERT_EQ(value, s.getUTCDate());
}

TEST(BuilderTest, UTCDateMin) {
  int64_t const value = INT64_MIN;
  Builder b;
  b.add(Value(value, ValueType::UTCDate));

  Slice s(b.start());
  ASSERT_EQ(0x1cU, s.head());
  ASSERT_TRUE(s.isUTCDate());
  ASSERT_EQ(9UL, s.byteSize());
  ASSERT_EQ(value, s.getUTCDate());
}

TEST(BuilderTest, UTCDateMax) {
  int64_t const value = INT64_MAX;
  Builder b;
  b.add(Value(value, ValueType::UTCDate));

  Slice s(b.start());
  ASSERT_EQ(0x1cU, s.head());
  ASSERT_TRUE(s.isUTCDate());
  ASSERT_EQ(9UL, s.byteSize());
  ASSERT_EQ(value, s.getUTCDate());
}

TEST(BuilderTest, ID) {
  // This is somewhat tautological, nevertheless...
  static uint8_t const correctResult[]
    = { 0xf1, 0x2b, 0x78, 0x56, 0x34, 0x12,
        0x45, 0x02, 0x03, 0x05, 0x08, 0x0d };

  Builder b;
  uint8_t* p = b.add(ValuePair(sizeof(correctResult), ValueType::Custom));
  memcpy(p, correctResult, sizeof(correctResult));
  uint8_t* result = b.start();
  ValueLength len = b.size();

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, AddOnNonArray) {
  Builder b;
  b.add(Value(ValueType::Object));
  EXPECT_VELOCYPACK_EXCEPTION(b.add(Value(true)), Exception::BuilderNeedOpenArray);
}

TEST(BuilderTest, AddOnNonObject) {
  Builder b;
  b.add(Value(ValueType::Array));
  EXPECT_VELOCYPACK_EXCEPTION(b.add("foo", Value(true)), Exception::BuilderNeedOpenObject);
}

TEST(BuilderTest, StartCalledOnOpenObject) {
  Builder b;
  b.add(Value(ValueType::Object));
  EXPECT_VELOCYPACK_EXCEPTION(b.start(), Exception::BuilderNotSealed);
}

TEST(BuilderTest, StartCalledOnOpenObjectWithSubs) {
  Builder b;
  b.add(Value(ValueType::Array));
  b.add(Value(ValueType::Array));
  b.add(Value(1));
  b.add(Value(2));
  b.close();
  EXPECT_VELOCYPACK_EXCEPTION(b.start(), Exception::BuilderNotSealed);
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

