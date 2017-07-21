/*
"Monoque Data Structure Implementation" http://rpnx.net/monoque.pdf
Copyright (c) 2017 Ryan P. Nicholl <r.p.nicholl@gmail.com> http://rpnx.net/
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
#include <tuple>
#include <atomic>
/*
  Like vector, but non-contiguous and has worst-case O(1) push_back and worst-case O(1) indexing.
  This is just a simple proof of concept at the present time. It is not a complete STL compatible
  container yet.
  
  TODO: Allocator Support, Iterator Support, STL Container Support.

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



namespace rpnx{
template <typename T, typename Allocator = std::allocator<T> >
class monoque
{
  size_t size_pv;
  std::array<T*, sizeof(T*)*8>   data_pv  ;

  //size_t cap;
private:
 
  static inline size_t index1_pv(size_t n) __attribute__((always_inline))  
  {
#ifdef __x86_64__ 
    if (rpnx_unlikely(n==0)) return 0;
    else
      {
        size_t i;
        asm("bsrq %1,%0\n" : "=r"(i) : "r"(n));
        return i;
      }
#else
    int i = 0; while ((1 << i) <= n) i++;
    return i == 0 ? 0 : i - 1;
#endif   
  }

  static inline size_t sizeat_pv(size_t at)  __attribute__((always_inline))
  {
    if (rpnx_unlikely(at <= 1)) return 2;
    else return (size_t(1) << index1_pv(at));
  }
 

  static inline size_t index2_pv(size_t n) __attribute__((always_inline))
  {
#if defined(__x86_64__) //&& !defined(__clang__)
    // No idea why, but for some reason this is slow as fuck in clang -O2.
    if (rpnx_unlikely(n<=1)) return n&1;
    else
      {
        size_t i;
        asm("bsrq %1,%0\n" : "=r"(i) : "r"(n));
        return n ^ (size_t(1) << i);
      }
#else
  return n & (sizeat_pv(n)-1);
#endif
  }
  
  static inline std::tuple<size_t, size_t> index_pv(size_t at)  __attribute__((always_inline))
  {
    return {index1_pv(at), index2_pv(at)};
  }

  
public:
  using value_type = T;
  using allocator_type = Allocator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using referece = value_type&;
  using const_reference = value_type const &;
  using pointer = typename std::allocator_traits<Allocator>::pointer;
  using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

  class iterator
  {
    monoque *m;
    size_type i;
  public:
    iterator()
      : m (nullptr), i(0)
    {
    }

    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer =typename std::allocator_traits<Allocator>::pointer;

    iterator(const iterator&)=default;
    iterator(iterator &&)=default;

    iterator & operator=(iterator const&)=default;
    iterator & operator=(iterator &&)=default;

    T  & operator*() const
    {
      return (*m)[i];
    }

    iterator & operator++()
    {
      i++;
      return *this;
    }

    iterator operator++(int)
    {
      iterator copy = *this;
      i++;
      return copy;
    }

    iterator & operator +=(difference_type n)
    {
      i+=n;
      return *this;
    }

    iterator & operator -=(difference_type n)
    {
      i-=n;
      return *this;
    }

    value_type & operator[](difference_type n) const
    {
      return *(*this+i);
    }

    difference_type operator - (iterator const & other) const
    {
      return i - other.i;
    }

    iterator operator +(difference_type n) const
    {
      iterator copy = *this;
      copy.i += n;
      return copy;
    }


    iterator operator -(difference_type n) const
    {
      iterator copy = *this;
      copy.i -= n;
      return copy;
    }

    bool operator ==(iterator const &o) const
    {
      return m == o.m && i == o.i;
    }


    bool operator !=(iterator const &o) const
    {
      return m != o.m && i != o.i;
    }

    bool operator <(iterator const &o) const
    {
      return i < o.i;
    }
    
    bool operator <=(iterator const &o) const
    {
      return i <= o.i;
    }

    bool operator >(iterator const &o) const
    {
      return i > o.i;
    }
    
    bool operator >=(iterator const &o) const
    {
      return i >= o.i;
    }
  };

  using reverse_iterator = std::reverse_iterator<iterator>;

  monoque()
    : size_pv(0)
  {
    for (auto & x: data_pv) x = nullptr;
  }

  ~monoque()
  {
    if (!std::is_trivially_destructible<T>::value) while (size() != 0) pop_back();

    for (auto & x: data_pv) if (x != nullptr) delete[] (char*) x;
  }

  inline T & operator[](size_t at)  //__attribute__((always_inline)) 
  {

    using namespace std;
#if  defined(__clang__)
     std::atomic_signal_fence(memory_order_seq_cst);
#endif

    size_t index1;
    size_t index2;

    index1 = index1_pv(at);
    index2 = index2_pv(at);
    //    index2 = index2_pv(at);


    return data_pv[index1][index2];
  }


  inline T const & operator[](size_t at) const //  __attribute__((always_inline)) 
  {
    using namespace std;

#if defined(__clang__)
    std::atomic_signal_fence(memory_order_seq_cst);
#endif

    size_t index1;
    size_t index2;
    
    tie(index1, index2) = index_pv(at);
    //    tie(index2) = index2_pv(at);

    return data_pv[index1][index2];
  }


  size_t size() const
  {
    return size_pv;
  }

  inline void push_back (T t)
  {
    if (index2_pv(size_pv) == 0)
      {
        if (!data_pv[index1_pv(size_pv)]) data_pv[index1_pv(size_pv)] = (T*) new char[sizeof(T)*sizeat_pv(size_pv)];
      }

    new ((void*)(((T*)data_pv[index1_pv(size_pv)])+index2_pv(size_pv))) T(std::move(t));
    size_pv++;
  }

  void pop_back()
  {
    this->operator[](size_pv-1).T::~T();
    size_pv--;
  }

  void shink_to_fit()
  {
    size_t mindex = 0;
    if (size_pv != 0) mindex = index1_pv(size_pv-1) + 1;
    for (size_t i = mindex; i < sizeof(T*)*8; i++)
      {
        if (data_pv[i] != nullptr) 
          {
            delete[] (char*) data_pv[i];
            data_pv[i] = nullptr;
          }
        else break;
      }    
  }

  
};

}//namespace rpnx

#endif
