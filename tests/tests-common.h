
#include "Jason.h"
#include "JasonBuffer.h"
#include "JasonBuilder.h"
#include "JasonDump.h"
#include "JasonException.h"
#include "JasonOptions.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

#include "gtest/gtest.h"

using namespace arangodb::jason;

#ifdef __GNUC__
#define JASON_UNUSED __attribute__ ((unused))
#else
#define JASON_UNUSED /* unused */
#endif

// helper for catching Jason-specific exceptions
#define EXPECT_JASON_EXCEPTION(operation, code) \
  try {                                         \
    operation;                                  \
    EXPECT_FALSE(true);                         \
  }                                             \
  catch (JasonException const& ex) {            \
    EXPECT_EQ(code, ex.errorCode());            \
  }                                             \
  catch (...) {                                 \
    EXPECT_FALSE(true);                         \
  } 

// don't complain if this function is not called
static void dumpDouble (double, uint8_t*) JASON_UNUSED;
  
static void dumpDouble (double x, uint8_t* p) {
  uint64_t u;
  memcpy(&u, &x, sizeof(double));
  for (size_t i = 0; i < 8; i++) {
    p[i] = u & 0xff;
    u >>= 8;
  }
}

// don't complain if this function is not called
static void checkDump (JasonSlice, std::string const&) JASON_UNUSED;

static void checkDump (JasonSlice s, std::string const& knownGood) {
  JasonCharBuffer buffer;
  JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
  dumper.dump(s);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(knownGood, output);
}

// don't complain if this function is not called
static void checkBuild (JasonSlice, JasonType, JasonLength) JASON_UNUSED;

// With the following function we check type determination and size
// of the produced Jason value:
static void checkBuild (JasonSlice s, JasonType t, JasonLength byteSize) {
  ASSERT_EQ(t, s.type());
  ASSERT_TRUE(s.isType(t));
  JasonType other = (t == JasonType::String) ? JasonType::Int
                                             : JasonType::String;
  ASSERT_FALSE(s.isType(other));
  ASSERT_FALSE(other == s.type());

  ASSERT_EQ(byteSize, s.byteSize());

  switch (t) {
    case JasonType::None:
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
    case JasonType::Null:
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
    case JasonType::Bool:
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
    case JasonType::Double:
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
    case JasonType::Array:
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
    case JasonType::Object:
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
    case JasonType::External:
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
    case JasonType::UTCDate:
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
    case JasonType::Int:
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
    case JasonType::UInt:
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
    case JasonType::SmallInt:
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
    case JasonType::String:
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
    case JasonType::Binary:
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
    case JasonType::BCD:
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
    case JasonType::MinKey:
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
    case JasonType::MaxKey:
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
    case JasonType::Custom:
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
