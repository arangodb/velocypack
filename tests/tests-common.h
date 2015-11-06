
#include "velocypack/velocypack-common.h"
#include "velocypack/Buffer.h"
#include "velocypack/Builder.h"
#include "velocypack/Dump.h"
#include "velocypack/Exception.h"
#include "velocypack/Iterator.h"
#include "velocypack/Options.h"
#include "velocypack/Parser.h"
#include "velocypack/Slice.h"
#include "velocypack/Value.h"
#include "velocypack/ValueType.h"

#include "gtest/gtest.h"

using namespace arangodb::velocypack;

// helper for catching VPack-specific exceptions
#define EXPECT_VELOCYPACK_EXCEPTION(operation, code) \
  try {                                         \
    (operation);                                \
    EXPECT_FALSE(true);                         \
  }                                             \
  catch (Exception const& ex) {                 \
    EXPECT_EQ(code, ex.errorCode());            \
  }                                             \
  catch (...) {                                 \
    EXPECT_FALSE(true);                         \
  } 

// don't complain if this function is not called
static void dumpDouble (double, uint8_t*) VELOCYPACK_UNUSED;
  
static void dumpDouble (double x, uint8_t* p) {
  uint64_t u;
  memcpy(&u, &x, sizeof(double));
  for (size_t i = 0; i < 8; i++) {
    p[i] = u & 0xff;
    u >>= 8;
  }
}

// don't complain if this function is not called
static void checkDump (Slice, std::string const&) VELOCYPACK_UNUSED;

static void checkDump (Slice s, std::string const& knownGood) {
  CharBuffer buffer;
  BufferDumper dumper(buffer);
  dumper.dump(s);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(knownGood, output);
}

// don't complain if this function is not called
static void checkBuild (Slice, ValueType, ValueLength) VELOCYPACK_UNUSED;

// With the following function we check type determination and size
// of the produced VPack value:
static void checkBuild (Slice s, ValueType t, ValueLength byteSize) {
  ASSERT_EQ(t, s.type());
  ASSERT_TRUE(s.isType(t));
  ValueType other = (t == ValueType::String) ? ValueType::Int
                                             : ValueType::String;
  ASSERT_FALSE(s.isType(other));
  ASSERT_FALSE(other == s.type());

  ASSERT_EQ(byteSize, s.byteSize());

  switch (t) {
    case ValueType::None:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      break;
    case ValueType::Null:
      ASSERT_TRUE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::Bool:
      ASSERT_FALSE(s.isNull());
      ASSERT_TRUE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::Double:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_TRUE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::Array:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_TRUE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::Object:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_TRUE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::External:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_TRUE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::UTCDate:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_TRUE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::Int:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_TRUE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::UInt:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_TRUE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::SmallInt:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_TRUE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::String:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_TRUE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::Binary:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_TRUE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::BCD:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_TRUE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::MinKey:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_TRUE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::MaxKey:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_TRUE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case ValueType::Custom:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_TRUE(s.isCustom());
      break;
  }
}
