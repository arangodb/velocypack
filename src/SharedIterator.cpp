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

#include <velocypack/SharedIterator.h>

using namespace arangodb;
using namespace arangodb::velocypack;

SharedArrayIterator::SharedArrayIterator(SharedSlice&& slice)
    : _slice(std::move(slice)), _iterator(_slice.slice()) {}

SharedArrayIterator::SharedArrayIterator(SharedSlice const& slice)
    : _slice(slice), _iterator(_slice.slice()) {}

SharedArrayIterator& SharedArrayIterator::operator++() {
  iterator().operator++();
  return *this;
}

SharedArrayIterator SharedArrayIterator::operator++(int) & {
  SharedArrayIterator result(*this);
  this->operator++();
  return result;
}

bool SharedArrayIterator::operator!=(SharedArrayIterator const& other) const noexcept {
  return iterator() != other.iterator();
}

SharedSlice SharedArrayIterator::operator*() const {
  return alias(iterator().operator*());
}

SharedArrayIterator SharedArrayIterator::begin() const {
  auto it = SharedArrayIterator(*this);
  it.iterator().begin();
  return it;
}

SharedArrayIterator SharedArrayIterator::end() const {
  auto it = SharedArrayIterator(*this);
  it.iterator().end();
  return it;
}

bool SharedArrayIterator::valid() const noexcept { return iterator().valid(); }

SharedSlice SharedArrayIterator::value() const { return operator*(); }

void SharedArrayIterator::next() { operator++(); }

ValueLength SharedArrayIterator::index() const noexcept {
  return iterator().index();
}

ValueLength SharedArrayIterator::size() const noexcept {
  return iterator().size();
}

bool SharedArrayIterator::isFirst() const noexcept {
  return iterator().isFirst();
}

bool SharedArrayIterator::isLast() const noexcept {
  return iterator().isLast();
}

void SharedArrayIterator::forward(ValueLength count) {
  iterator().forward(count);
}

void SharedArrayIterator::reset() { iterator().reset(); }

SharedSlice& SharedArrayIterator::sharedSlice() noexcept { return _slice; }

SharedSlice const& SharedArrayIterator::sharedSlice() const noexcept {
  return _slice;
}

ArrayIterator& SharedArrayIterator::iterator() noexcept { return _iterator; }

ArrayIterator const& SharedArrayIterator::iterator() const noexcept {
  return _iterator;
}

SharedSlice SharedArrayIterator::alias(Slice slice) const noexcept {
  return SharedSlice(sharedSlice(), slice);
}

SharedObjectIterator::ObjectPair::ObjectPair(SharedSlice key, SharedSlice value) noexcept
    : key(std::move(key)), value(std::move(value)) {}

SharedObjectIterator::SharedObjectIterator(SharedSlice&& slice, bool useSequentialIteration)
    : _slice(std::move(slice)), _iterator(_slice.slice(), useSequentialIteration) {}

SharedObjectIterator::SharedObjectIterator(SharedSlice const& slice, bool useSequentialIteration)
    : _slice(slice), _iterator(_slice.slice(), useSequentialIteration) {}

SharedObjectIterator& SharedObjectIterator::operator++() {
  iterator().operator++();
  return *this;
}

SharedObjectIterator SharedObjectIterator::operator++(int) & {
  SharedObjectIterator result(*this);
  iterator().operator++();
  return result;
}

bool SharedObjectIterator::operator!=(SharedObjectIterator const& other) const {
  return iterator() != other.iterator();
}

SharedObjectIterator::ObjectPair SharedObjectIterator::operator*() const {
  auto pair = iterator().operator*();
  return ObjectPair(alias(pair.key), alias(pair.value));
}

SharedObjectIterator SharedObjectIterator::begin() const {
  auto it = SharedObjectIterator(*this);
  it.iterator().begin();
  return it;
}

SharedObjectIterator SharedObjectIterator::end() const {
  auto it = SharedObjectIterator(*this);
  it.iterator().end();
  return it;
}

bool SharedObjectIterator::valid() const noexcept { return iterator().valid(); }

SharedSlice SharedObjectIterator::key(bool translate) const {
  return alias(iterator().key(translate));
}

SharedSlice SharedObjectIterator::value() const {
  return alias(iterator().value());
}

void SharedObjectIterator::next() { operator++(); }

ValueLength SharedObjectIterator::index() const noexcept {
  return iterator().index();
}

ValueLength SharedObjectIterator::size() const noexcept {
  return iterator().size();
}

bool SharedObjectIterator::isFirst() const noexcept {
  return iterator().isFirst();
}

bool SharedObjectIterator::isLast() const noexcept {
  return iterator().isLast();
}

void SharedObjectIterator::reset() { iterator().reset(); }

SharedSlice& SharedObjectIterator::sharedSlice() noexcept { return _slice; }

SharedSlice const& SharedObjectIterator::sharedSlice() const noexcept {
  return _slice;
}

ObjectIterator& SharedObjectIterator::iterator() noexcept { return _iterator; }

ObjectIterator const& SharedObjectIterator::iterator() const noexcept {
  return _iterator;
}

SharedSlice SharedObjectIterator::alias(Slice slice) const noexcept {
  return SharedSlice(sharedSlice(), slice);
}
