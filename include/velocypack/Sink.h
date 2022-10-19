////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2020 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
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
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include "velocypack/velocypack-common.h"
#include "velocypack/Buffer.h"

namespace arangodb::velocypack {

struct Sink {
  Sink() {}
  Sink(Sink const&) = delete;
  Sink& operator=(Sink const&) = delete;

  virtual ~Sink() = default;
  virtual void push_back(char c) = 0;
  virtual void append(std::string const& p) = 0;
  virtual void append(char const* p) = 0;
  virtual void append(char const* p, ValueLength len) = 0;
  virtual void reserve(ValueLength len) = 0;
};

template<typename T>
struct ByteBufferSinkImpl final : public Sink {
  explicit ByteBufferSinkImpl(Buffer<T>* buffer) : _buffer(buffer) {}

  void push_back(char c) override final { _buffer->push_back(c); }

  void append(std::string const& p) override final {
    _buffer->append(p.c_str(), p.size());
  }

  void append(char const* p) override final { _buffer->append(p, strlen(p)); }

  void append(char const* p, ValueLength len) override final {
    _buffer->append(p, len);
  }

  void reserve(ValueLength len) override final { _buffer->reserve(len); }

 private:
  Buffer<T>* _buffer;
};

typedef ByteBufferSinkImpl<char> CharBufferSink;

template<typename T>
struct StringSinkImpl final : public Sink {
  explicit StringSinkImpl(T* buffer) : _buffer(buffer) {}

  void push_back(char c) override final { _buffer->push_back(c); }

  void append(std::string const& p) override final { _buffer->append(p); }

  void append(char const* p) override final { _buffer->append(p, strlen(p)); }

  void append(char const* p, ValueLength len) override final {
    _buffer->append(p, checkOverflow(len));
  }

  void reserve(ValueLength len) override final {
    ValueLength length = len + _buffer->size();
    if (length <= _buffer->capacity()) {
      return;
    }
    _buffer->reserve(checkOverflow(len));
  }

 private:
  T* _buffer;
};

typedef StringSinkImpl<std::string> StringSink;

// a sink with an upper bound for the generated output value
template<typename T>
struct SizeConstrainedStringSinkImpl final : public Sink {
  explicit SizeConstrainedStringSinkImpl(T* buffer, ValueLength maxLength) 
      : _buffer(buffer), _maxLength(maxLength), _overflowed(false) {}

  void push_back(char c) override final { 
    if (_buffer->size() < _maxLength) {
      _buffer->push_back(c); 
    } else {
      _overflowed = true;
    }
  }

  void append(std::string const& p) override final {
    append(p.data(), p.size());
  }

  void append(char const* p) override final { append(p, strlen(p)); }

  void append(char const* p, ValueLength len) override final {
    if (_buffer->size() < _maxLength) {
      ValueLength total = checkOverflow(_buffer->size() + checkOverflow(len));
      if (total <= _maxLength) {
        _buffer->append(p, len);
        return;
      }
      ValueLength left = _maxLength - _buffer->size();
      if (len > left) {
        len = left;
      }
      _buffer->append(p, len);
    }
    _overflowed = true;
  }

  void reserve(ValueLength len) override final {
    ValueLength total = checkOverflow(_buffer->size() + checkOverflow(len));
    if (total <= _buffer->capacity()) {
      return;
    }
    ValueLength left = _maxLength - _buffer->size();
    if (len > left) {
      len = left;
    }
    _buffer->reserve(checkOverflow(len));
  }

  bool overflowed() const noexcept { return _overflowed; }

 private:
  T* _buffer;
  std::size_t const _maxLength;
  bool _overflowed;
};

typedef SizeConstrainedStringSinkImpl<std::string> SizeConstrainedStringSink;

// only tracks the length of the generated output
struct StringLengthSink final : public Sink {
  StringLengthSink() : _length(0) {}

  void push_back(char) override final { ++_length; }

  void append(std::string const& p) override final { _length += p.size(); }

  void append(char const* p) override final { _length += strlen(p); }

  void append(char const*, ValueLength len) override final { _length += len; }

  void reserve(ValueLength) override final {}

  ValueLength length() const noexcept { return _length; }

 private:
  ValueLength _length;
};

template<typename T>
struct StreamSinkImpl final : public Sink {
  explicit StreamSinkImpl(T* stream) : _stream(stream) {}

  void push_back(char c) override final { *_stream << c; }

  void append(std::string const& p) override final { *_stream << p; }

  void append(char const* p) override final {
    _stream->write(p, static_cast<std::streamsize>(strlen(p)));
  }

  void append(char const* p, ValueLength len) override final {
    _stream->write(p, static_cast<std::streamsize>(len));
  }

  void reserve(ValueLength) override final {}

 private:
  T* _stream;
};

typedef StreamSinkImpl<std::ostringstream> StringStreamSink;
typedef StreamSinkImpl<std::ofstream> OutputFileStreamSink;

}  // namespace arangodb::velocypack

using VPackSink = arangodb::velocypack::Sink;
using VPackCharBufferSink = arangodb::velocypack::CharBufferSink;
using VPackStringSink = arangodb::velocypack::StringSink;
using VPackStringLengthSink = arangodb::velocypack::StringLengthSink;
using VPackStringStreamSink = arangodb::velocypack::StringStreamSink;
using VPackOutputFileStreamSink = arangodb::velocypack::OutputFileStreamSink;
