#pragma once

namespace mdo {

template <typename T>
inline T LoadRelaxed(const std::atomic<T>& a) noexcept {
  return a.load(std::memory_order_relaxed);
}

template <typename T>
inline T LoadAcqRel(const std::atomic<T>& a) noexcept {
  return a.load(std::memory_order_acq_rel);
}

template <typename T>
inline T LoadAcquire(const std::atomic<T>& a) noexcept {
  return a.load(std::memory_order_acquire);
}

template <typename T>
inline T LoadRelease(const std::atomic<T>& a) noexcept {
  return a.load(std::memory_order_release);
}

template <typename T>
inline T LoadConsume(const std::atomic<T>& a) noexcept {
  return a.load(std::memory_order_consume);
}

template <typename T>
inline T LoadSeqCst(const std::atomic<T>& atomic) noexcept {
  return atomic.load(std::memory_order_seq_cst);
}



template <typename T>
inline void StoreRelaxed(std::atomic<T>& a, const T& v) noexcept {
  return a.store(v, std::memory_order_relaxed);
}

template <typename T>
inline void StoreAcqRel(std::atomic<T>& a, const T& v) noexcept {
  return a.store(v, std::memory_order_acq_rel);
}

template <typename T>
inline void StoreAcquire(std::atomic<T>& a, const T& v) noexcept {
  return a.store(v, std::memory_order_acquire);
}

template <typename T>
inline void StoreRelease(std::atomic<T>& a, const T& v) noexcept {
  return a.store(v, std::memory_order_release);
}

template <typename T>
inline void StoreConsume(std::atomic<T>& a, const T& v) noexcept {
  return a.store(v, std::memory_order_consume);
}

template <typename T>
inline void StoreSeqCst(std::atomic<T>& atomic, const T& v) noexcept {
  return atomic.store(v, std::memory_order_seq_cst);
}

}