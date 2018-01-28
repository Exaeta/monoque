/*
Monoque Data Structure

Copyright (c) 2017, 2018 Ryan P. Nicholl <exaeta@protonmail.com> http://rpnx.net/
 -- Please let me know if you find this structure useful, thanks! 
 
All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef RPNX_MONOQUE_HH
#define RPNX_MONOQUE_HH

#include <array>
#include <assert.h>
#include <atomic>
#include <inttypes.h>
#include <iterator>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <tuple>
/*
  Like vector, but non-contiguous and has worst-case O(1) push_back and
  worst-case O(1) indexing.
  This is just a simple proof of concept at the present time. It is not a
  complete STL compatible container yet.

  TODO: 
  Works in progress:
      Allocator Support, Iterator Support, STL Container Support.

 */
#ifndef rpnx_expect
#if false
#define rpnx_expect(x, y) x
#else
#define rpnx_expect(x, y) __builtin_expect(x, y)
#endif
#define rpnx_likely(x) rpnx_expect(x, 1)
#define rpnx_unlikely(x) rpnx_expect(x, 0)
#endif

namespace rpnx {
template <typename T, typename Allocator = std::allocator<T>> class monoque : private Allocator {
public:
  using value_type = T;
  using allocator_type = Allocator;
  using const_reference = typename allocator_type::const_reference;
  using pointer = typename allocator_type::pointer;
  using const_pointer = typename allocator_type::const_pointer;
  using size_type = typename allocator_type::size_type;
  using reference = typename Allocator::reference;

  static_assert(std::is_same<size_type, size_t>::value, "currently unsupported");
  static_assert(std::is_same<reference, T &>::value, "wut");

private:
  size_t size_pv;
  std::array<pointer, sizeof(T *) * 8> data_pv;

  static inline size_t bfill(size_t n) {
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    if (sizeof(size_t) > 4)
      n |= n >> 32;
    return n;
  }

private:
  static inline size_t index1_pv(size_t n) {
#if defined(__GNUC__) && SIZE_MAX == 18446744073709551615ull && ULONG_LONG_MAX == SIZE_MAX
    return 63 - __builtin_clzll(n | 1);
#elif defined(__GNUC__) && SIZE_MAX == 4294967295ull && UINT_MAX == SIZE_MAX
    return 31 - __builtin_clz(n | 1);
#elif defined(__x86_64__)
#warning Fallback to x64 assembly
    if (n == 0)
      return 0;
    else {
      size_t i;
      asm("bsrq %1,%0\n" : "=r"(i) : "r"(n));
      return i;
    }
#else
#warning Very slow generic implementation
    if (n == 0)
      return 0;
    size_t n2 = 0;
    while (n != 1) { n >>=1; n2 ++; }
    return n2;
   size_t a = bfill(n);
   a = a & ~(a >> 1);
   size_t i = 0;
   if (a & 0b10101010101010101010101010101010) i |= 1;
   if (a & 0b11001100110011001100110011001100) i |= 2;
   if (a & 0b11110000111100001111000011110000) i |= 4;
   if (a & 0b11111111000000001111111100000000) i |= 8;
   if (a & 0b11111111111111110000000000000000) i |= 16;
#if SIZE_MAX > 4294967295ull
   if (a & 0b1111111111111111111111111111111100000000000000000000000000000000ull) i |= 32;
#endif
    return i;
#endif
  }

  static inline size_t sizeat_pv(size_t at) {
#if defined(__GNUC__) && SIZE_MAX == 18446744073709551615ull && ULONG_LONG_MAX == SIZE_MAX
    static_assert(sizeof(unsigned long long) == 8, "This is a bug.");

    return 1 << (64 - __builtin_clzll((at >> 1) | 1));
#elif defined(__GNUC__) && SIZE_MAX == 4294967295ull && UINT_MAX == SIZE_MAX
    return 1 << (32 - __builtin_clz((at >> 1) | 1));
#elif defined(__x86_64__)
#warning Fell back to inline assembly?
    if (at == 0)
      return 2;
    size_t i;
    asm("bsrq %1,%0\n" : "=r"(i) : "r"(at));
    return size_t(1) << i;
#else
#warning Generic implementation may be slow.
    if (at == 0)
      return 2;
    else
      return bfill(at >> 1) + 1;
#endif
  }

  static inline size_t index2_pv(size_t n) {
#if defined(__GNUC__) && SIZE_MAX == 18446744073709551615ull && ULONG_LONG_MAX == SIZE_MAX
    static_assert(sizeof(unsigned long long) == 8, "This is a bug.");
    return n & ((1 << (63 - __builtin_clzll(n | 2))) - 1);
#elif defined(__GNUC__) && SIZE_MAX == 4294967295ull && UINT_MAX == SIZE_MAX
    static_assert(sizeof(unsigned long long) == 8, "This is a bug.");
    return n & ((1 << (31 - __builtin_clz(n | 2))) - 1);
#elif defined(__x86_64__)
    if (n <= 1)
      return n & 1;
    else {
      size_t i;
      asm("bsrq %1,%0\n" : "=r"(i) : "r"(n));
      return n ^ (size_t(1) << i);
    }
#else
#warning Generic implementation used.
    return n & (bfill(n) >> 1);
#endif
  }

  static inline std::tuple<size_t, size_t> index_pv(size_t at) { return std::tuple<size_t, size_t>{index1_pv(at), index2_pv(at)}; }

  void check_cleanup() {}

public:
  class const_iterator {
  public:
    // template <typename T, Allocator>
    friend class monoque<T, Allocator>;
    friend class monoque<T, Allocator>::iterator;

  protected:
    monoque<T, Allocator> const *m;
    size_type i;

  public:
    const_iterator() : m(nullptr), i(0) {}

    using value_type = monoque<T, Allocator>::value_type;
    using difference_type = ssize_t;
    using pointer = T const *;
    using reference = T const &;
    using category = std::random_access_iterator_tag;

    const_iterator(const const_iterator &) = default;
    const_iterator(const_iterator &&) = default;

    const_iterator &operator=(const_iterator const &) = default;
    const_iterator &operator=(const_iterator &&) = default;

    value_type const &operator*() const { return (*m)[i]; }

    const_iterator &operator++() {
      i++;
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator copy = *this;
      i++;
      return copy;
    }

    const_iterator &operator--() {
      i--;
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator copy = *this;
      i--;
      return copy;
    }

    const_iterator &operator+=(difference_type n) {
      i += n;
      return *this;
    }

    const_iterator &operator-=(difference_type n) {
      i -= n;
      return *this;
    }

    const_iterator operator+(difference_type n) const {
      const_iterator copy = *this;
      copy.i += n;
      return copy;
    }

    const_iterator operator-(difference_type n) const {
      const_iterator copy = *this;
      copy.i -= n;
      return copy;
    }

    bool operator==(const_iterator const &o) const { return m == o.m && i == o.i; }

    bool operator!=(const_iterator const &o) const { return m != o.m || i != o.i; }

    bool operator<(const_iterator const &o) const { return i < o.i; }

    bool operator<=(const_iterator const &o) const { return i <= o.i; }

    bool operator>(const_iterator const &o) const { return i > o.i; }

    bool operator>=(const_iterator const &o) const { return i >= o.i; }

    value_type const &operator[](difference_type n) const { return (*m)[i + n]; }

    difference_type operator-(const_iterator const &other) const { return i - other.i; }
  };

  class iterator : public const_iterator {
  public:
    using value_type = typename monoque<T, Allocator>::value_type;
    using difference_type = ssize_t;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::random_access_iterator_tag;

    iterator() : const_iterator() {}
    iterator(iterator const &) = default;
    iterator(const_iterator const &) = delete;

    inline reference operator*() const { return const_cast<reference>(this->const_iterator::operator*()); }

    inline pointer operator->() const { return &const_cast<reference>(this->const_iterator::operator*()); }

    iterator &operator++() {
      const_iterator::operator++();
      return *this;
    }

    iterator operator++(int) {
      iterator copy = *this;
      const_iterator::operator++(0);
      return copy;
    }

    iterator &operator--() {
      const_iterator::operator--();
      return *this;
    }

    iterator operator--(int) {
      iterator copy = *this;
      const_iterator::operator--(0);
      return copy;
    }

    iterator &operator+=(ssize_t n) {
      this->i += n;
      return *this;
    }

    iterator &operator-=(ssize_t n) {
      this->i -= n;
      return *this;
    }

    iterator operator+(difference_type n) const {
      iterator copy = *this;
      copy.i += n;
      return copy;
    }

    iterator operator-(difference_type n) const {
      iterator copy = *this;
      copy.i -= n;
      return copy;
    }

    value_type &operator[](difference_type n) const { return const_cast<T &>(const_iterator::operator[](n)); }

    difference_type operator-(const_iterator const &other) const { return this->i - other.i; }
  };

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  monoque() : Allocator(std::allocator<T>()), size_pv(0) {
    for (auto &x : data_pv)
      x = nullptr;
  }

  explicit monoque(allocator_type const &alloc) : allocator_type(alloc), size_pv(0) {

    for (auto &a : data_pv)
      a = nullptr;
  }

  explicit monoque(size_type n, allocator_type const &alloc = std::allocator<T>()) : monoque(alloc) { resize(n); }

  explicit monoque(size_type n, value_type const &val, allocator_type const &alloc = std::allocator<T>()) : monoque(alloc) {

    for (size_type i = 0; i != n; i++) {
      push_back(val);
    }
  }

  monoque(std::initializer_list<value_type> il, allocator_type const &alloc = std::allocator<T>()) : monoque(alloc) { assign(il.begin(), il.end()); }

  template <typename It> monoque(It begin, It end, allocator_type const &alloc = std::allocator<T>()) : monoque(alloc) {
    for (auto i = begin; i != end; i++) {
      push_back(*i);
    }
  }

  monoque(monoque<T, Allocator> const &other) : monoque(other.get_allocator()) {
    using namespace std;
    for (auto const &x : other)
      push_back(x);
  }

  monoque(monoque<T, Allocator> &&other) : monoque(other.get_allocator()) { swap(other); }

  monoque<T, Allocator> &operator=(monoque<T, Allocator> const &other) {

    monoque<T, Allocator> copy(get_allocator());
    copy.assign(other.begin(), other.end());
    swap(copy);
    return *this;
  }

  monoque<T, Allocator> &operator=(monoque<T, Allocator> &&other) {
    swap(other);
    return *this;
  }

  ~monoque() {
    if (!std::is_trivially_destructible<T>::value)
      while (size() != 0)
        pop_back();

    for (size_t i = 0; i < sizeof(void *) * 8; i++) {
      size_t sz = 1 << i;
      if (sz == 1)
        sz = 2;
      if (data_pv[i] != nullptr)
        Allocator::deallocate(data_pv[i], sz);
    }
  }

  inline T &operator[](size_t at) //__attribute__((always_inline))
  {

    using namespace std;

    size_t index1;
    size_t index2;

    tie(index1, index2) = index_pv(at);

    return data_pv[index1][index2];
  }

  reference back() { return this->operator[](size() - 1); }

  const_reference back() const { return this->operator[](size() - 1); }

  reference front() { return this->operator[](0); }

  const_reference front() const { return this->operator[](0); }

  inline T const &operator[](size_t at) const //  __attribute__((always_inline))
  {
    using namespace std;

    size_t index1;
    size_t index2;

    tie(index1, index2) = index_pv(at);

    return data_pv[index1][index2];
  }

  size_t size() const { return size_pv; }

  allocator_type const &get_allocator() const { return *this; }

  template <typename It> inline void assign(It begin, It end) {
    monoque<T, Allocator> obj(begin, end, get_allocator());
    swap(obj);
    return;
  }

  void assign(std::initializer_list<T> ilist) { assign(ilist.begin(), ilist.end()); }

  void assign(size_type count, const T &value) {
    clear();
    for (size_t i = 0; i < count; i++) {
      push_back(value);
    }
  }

  reference at(size_type pos) {
    if (!(pos < size()))
      throw std::out_of_range("nope.avi");
    return this->operator[](pos);
  }

  const_reference at(size_type pos) const {
    if (!(pos < size()))
      throw std::out_of_range("nope.avi");
    return this->operator[](pos);
  }

  void clear() {
    monoque<T, Allocator> obj(get_allocator());
    swap(obj);
  }

  bool empty() const { return size() == 0; }

  inline void resize(size_type n) {
    while (size() > n)
      pop_back();
    while (size() < n)
      push_back(value_type());
  }

  inline void push_back(T t) {
    using namespace std;

    size_t i1, i2, s;
    s = size_pv;
    tie(i1, i2) = index_pv(s);

    if (data_pv[i1] == nullptr) {
      data_pv[i1] = Allocator::allocate(sizeat_pv(s));
    }
    this->Allocator::construct(data_pv[i1] + i2, std::move(t));
    size_pv++;
  }

  void pop_back() {
    assert(size() >= 1);
    Allocator::destroy(&this->operator[](size_pv - 1));
    size_pv--;
  }

  void swap(monoque<T, Allocator> &other) {
    std::swap(static_cast<allocator_type &>(*this), static_cast<allocator_type &>(other));
    std::swap(data_pv, other.data_pv);
    std::swap(size_pv, other.size_pv);
  }

  template <typename... Ts> void emplace_back(Ts &&... ts) {
    using namespace std;

    size_t i1, i2, s;
    s = size_pv;
    tie(i1, i2) = index_pv(s);

    if (data_pv[i1] == nullptr) {
      data_pv[i1] = this->Allocator::allocate(sizeat_pv(s));
    }
    this->Allocator::construct(data_pv[i1] + i2, std::forward<Ts>(ts)...);
    size_pv++;
  }

  friend void swap(rpnx::monoque<T, Allocator> &a, rpnx::monoque<T, Allocator> &b) { a.swap(b); }

  void shink_to_fit() {
    size_t mindex = 0;
    if (size_pv != 0)
      mindex = index1_pv(size_pv - 1) + 1;
    for (size_t i = mindex; i < sizeof(T *) * 8; i++) {
      if (data_pv[i] != nullptr) {
        delete[](char *) data_pv[i];
        data_pv[i] = nullptr;
      } else
        break;
    }
  }

  inline iterator begin() {
    iterator it;
    it.i = 0;
    it.m = this;
    return it;
  }

  inline iterator end() {
    iterator it;
    it.i = size();
    it.m = this;
    return it;
  }

  const_iterator cbegin() const {
    const_iterator it;
    it.i = 0;
    it.m = this;
    return it;
  }

  const_iterator cend() const {
    const_iterator it;
    it.i = size();
    it.m = this;
    return it;
  }

  const_iterator end() const { return cend(); }
  const_iterator begin() const { return cbegin(); }
};

} // namespace rpnx

#endif
