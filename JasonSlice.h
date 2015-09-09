#ifndef JASON_SLICE_H
#define JASON_SLICE_H 1

#include "JasonType.h"

namespace triagens {
  namespace basics {
    
    class JasonSlice {

      // This class provides read only access to a Jason value, it is
      // intentionally light-weight (only one pointer value), such that
      // it can easily be used to traverse larger Jason values.

        friend class JasonBuilder;

        uint8_t* _start;

      public:

        JasonSlice (uint8_t* start) : _start(start) {
        }

        // No destructor, does not take part in memory management,
        // standard copy, and move constructors, behaves like a pointer.

        JasonType type () {
          return JasonType::Null;
        }

        size_t size () {
          return 0ul;
        }

        uint8_t* start () {
          return _start;
        }

        bool isNull () {
          return false;
        }

        bool isBool () {
          return false;
        }

        bool getBool () {
          return false;
        }

        bool isDouble () {
          return false;
        }

        double getDouble () {
          return 0.0;
        }

        bool isArray () {
          return false;
        }

        JasonSlice at (size_t index) {
          return *this;
        }

        JasonSlice operator[] (size_t index) {
          return *this;
        }

        bool isObject () {
          return false;
        }

        size_t length () {
          return 0;
        }

        JasonSlice get (std::string& attribute) {
          return *this;
        }

        JasonSlice operator[] (std::string& attribute) {
          return *this;
        }

        bool isUTCDate () {
          return false;
        }

        uint64_t getUTCDate () {
          return 0ul;
        }

        bool isInt () {
          return false;
        }

        int64_t getInt (size_t bytes) {
          return 0l;
        }

        bool isUInt () {
          return false;
        }

        uint64_t getUInt (size_t& bytes) {
          return 0ul;
        }

        bool isString () {
          return false;
        }

        char* getString (size_t& length) {
          return nullptr;
        }

        std::string copyString () {
          return std::string("Hello");
        }

        bool isBinary () {
          return false;
        }

        uint8_t* getBinary (size_t& length) {
          return nullptr;
        }

        std::vector<uint8_t> copyBinary () {
          return std::vector<uint8_t>();
        }

        void toJsonString (std::string& out) {
        }
    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
