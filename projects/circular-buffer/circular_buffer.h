#pragma once

#include <cassert>
#include <cstddef>
#include <memory>

//!
//! A CircularBuffer is intended to store POD-like types in fixed-size flat memory area connected end-to-end.
//!
template <typename T>
class CircularBuffer {
 public:
  class Iterator {
   private:
    Iterator(T* buf, size_t head, size_t tail, size_t max_size, bool full)
        : buf_{buf}, head_{head}, tail_{tail}, max_size_{max_size}, full_{full} {}

    T& operator*() noexcept {
      return buf_[tail_];
    }

    Iterator& operator++() noexcept {
      if (full_) {
        tail_ = (tail_ + 1) % max_size_;
        return *this;
      }

      const bool isEmpty = !full_ && head_ == tail_;

      if (isEmpty) {
        return *this;
      }

      tail_ = (tail_ + 1) % head_;
      return *this;
    }

   private:
    T* buf_;
    size_t head_;
    size_t tail_;
    const size_t max_size_;
    const bool full_;
  };

  explicit CircularBuffer(size_t size) : max_size_{size}, buf_{new T[size]}, head_{}, tail_{}, full_{} {}

  CircularBuffer& operator=(CircularBuffer&& other) = delete;
  CircularBuffer& operator=(const CircularBuffer& other) = delete;

  CircularBuffer(CircularBuffer&& other) noexcept
      : max_size_(other.max_size_), buf_{other.buf_}, head_{other.head_}, tail_{other.tail_}, full_{other.full_} {
    other.buf_ = nullptr;
  }

  CircularBuffer(const CircularBuffer& other)
      : max_size_(other.max_size_), buf_{new T[max_size_]}, head_{other.head_}, tail_{other.tail_}, full_{other.full_} {
    std::uninitialized_copy(other.buf_, other.buf_ + max_size_, buf_);
  }

  ~CircularBuffer() { delete[] buf_; }

  void Put(const T& item) noexcept {
    buf_[head_] = item;

    if (Full()) {
      tail_ = (tail_ + 1) % max_size_;
    }

    head_ = (head_ + 1) % max_size_;
    full_ = (head_ == tail_);
  }

  T Get() noexcept {
    T& val = buf_[tail_];
    tail_ = (tail_ + 1) % max_size_;
    full_ = false;
    return val;
  }

  T operator[](size_t index) const noexcept {
    assert(!Empty());
    assert(index < Size() && "index is out of range");

    size_t actualIndex = (tail_ + index) % Size();
    return buf_[actualIndex];
  }

  void Reset() noexcept {
    head_ = tail_;
    full_ = false;
  }

  bool Empty() const noexcept {
    return !Full() && (head_ == tail_);
  }

  bool Full() const noexcept {
    return full_;
  }

  size_t Capacity() const noexcept {
    return max_size_;
  }

  size_t Size() const noexcept {
    if (!Full()) {
      auto diff = head_ - tail_;
      return diff >= 0 ? diff : Capacity() - diff;
    }

    return Capacity();
  }

 private:
  const size_t max_size_;

  T* buf_;

  size_t head_;
  size_t tail_;

  bool full_;
};
