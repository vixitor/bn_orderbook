#include <array>
#include <atomic>
#include <cstddef>
template <typename T, std::size_t capacity>
class SPSC {
  std::atomic<std::size_t> head{0}, tail{0};
  std::array<T, capacity> ring_buffer;

 public:
  bool push(const T& data) {
    auto h = head.load(), t = tail.load();
    if (t - h >= capacity) {
      return false;
    }
    ring_buffer[t % capacity] = data;
    tail.store(t + 1);
    return true;
  }
  bool pop(T* data) {
    auto h = head.load(), t = tail.load();
    if (h == t) {
      return false;
    }
    *data = ring_buffer[h % capacity];
    head.store(h + 1);
    return true;
  }
};
