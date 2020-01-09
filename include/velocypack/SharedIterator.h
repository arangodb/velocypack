////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2020 ArangoDB GmbH, Cologne, Germany
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
/// @author Tobias Gödderz
////////////////////////////////////////////////////////////////////////////////

#ifndef SRC_SHAREDITERATOR_H
#define SRC_SHAREDITERATOR_H

#include <velocypack/Iterator.h>
#include <velocypack/SharedSlice.h>

#include <utility>

namespace arangodb::velocypack {

class SharedArrayIterator {
 public:
  SharedArrayIterator() = delete;

  explicit SharedArrayIterator(SharedSlice&& slice);
  explicit SharedArrayIterator(SharedSlice const& slice);

  // prefix ++
  SharedArrayIterator& operator++();

  // postfix ++
  SharedArrayIterator operator++(int) &;

  bool operator!=(SharedArrayIterator const& other) const noexcept;

  SharedSlice operator*() const;

  [[nodiscard]] SharedArrayIterator begin() const;

  [[nodiscard]] SharedArrayIterator end() const;

  [[nodiscard]] bool valid() const noexcept;

  [[nodiscard]] SharedSlice value() const;

  void next();

  [[nodiscard]] ValueLength index() const noexcept;

  [[nodiscard]] ValueLength size() const noexcept;

  [[nodiscard]] bool isFirst() const noexcept;

  [[nodiscard]] bool isLast() const noexcept;

  void forward(ValueLength count);

  void reset();

 private:
  SharedSlice& sharedSlice() noexcept;
  [[nodiscard]] SharedSlice const& sharedSlice() const noexcept;
  ArrayIterator& iterator() noexcept;
  [[nodiscard]] ArrayIterator const& iterator() const noexcept;
  [[nodiscard]] SharedSlice alias(Slice slice) const noexcept;

 private:
  SharedSlice _slice;
  ArrayIterator _iterator;
};

class SharedObjectIterator {
 public:
  struct ObjectPair {
    ObjectPair(SharedSlice key, SharedSlice value) noexcept;
    SharedSlice key;
    SharedSlice value;
  };

  SharedObjectIterator() = delete;

  explicit SharedObjectIterator(SharedSlice&& slice, bool useSequentialIteration = false);
  explicit SharedObjectIterator(SharedSlice const& slice, bool useSequentialIteration = false);

  // prefix ++
  SharedObjectIterator& operator++();

  // postfix ++
  SharedObjectIterator operator++(int) &;

  [[nodiscard]] bool operator!=(SharedObjectIterator const& other) const;

  [[nodiscard]] ObjectPair operator*() const;

  [[nodiscard]] SharedObjectIterator begin() const;

  [[nodiscard]] SharedObjectIterator end() const;

  [[nodiscard]] bool valid() const noexcept;

  [[nodiscard]] SharedSlice key(bool translate = true) const;

  [[nodiscard]] SharedSlice value() const;

  void next();

  [[nodiscard]] ValueLength index() const noexcept;

  [[nodiscard]] ValueLength size() const noexcept;

  [[nodiscard]] bool isFirst() const noexcept;

  [[nodiscard]] bool isLast() const noexcept;

  void reset();

 private:
  [[nodiscard]] SharedSlice& sharedSlice() noexcept;
  [[nodiscard]] SharedSlice const& sharedSlice() const noexcept;
  [[nodiscard]] ObjectIterator& iterator() noexcept;
  [[nodiscard]] ObjectIterator const& iterator() const noexcept;
  [[nodiscard]] SharedSlice alias(Slice slice) const noexcept;

 private:
  SharedSlice _slice;
  ObjectIterator _iterator;
};

}  // namespace arangodb::velocypack

#endif  // SRC_SHAREDITERATOR_H
