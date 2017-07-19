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

/*
  Like vector, but non-contiguous and has worst-case O(1) push_back and worst-case O(1) indexing.
  This is just a simple proof of concept at the present time. It is not a complete STL compatible
  container yet.
  
  TODO: Allocator Support, Iterator Support, STL Container Support.

 */
namespace rpnx{
template <typename T>
class monoque
{
  std::array<T*, sizeof(T*)*8> data_pv;
  size_t size_pv;
  //size_t cap;
private:
 
  static inline size_t index1_pv(size_t n)
  {
#ifdef __x86_64__
    size_t i;    
    asm("bsrq %1,%0" : "=r"(i) : "r"(n));
    return n == 0 ? 0 : i;
#else
    int i = 0; while ((1 << i) <= n) i++;
#endif
    return i == 0 ? 0 : i - 1;
  }

  static inline size_t sizeat_pv(size_t at)
  {
    return at <= 1 ? 2 : (1 << index1_pv(at));
  }
 
  static inline size_t index2_pv(size_t at)
  {
    return at & (sizeat_pv(at)-1);
  }
  
  static inline std::tuple<size_t, size_t> index_pv(size_t at)
  {
    return {index1_pv(at), index2_pv(at)};
  }

  
public:
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

  T & operator[](size_t at)
  {
    using namespace std;

    size_t index1;
    size_t index2;
    
    tie(index1, index2) = index_pv(at);

    return data_pv[index1][index2];
  }


  T const & operator[](size_t at) const
  {
    using namespace std;

    size_t index1;
    size_t index2;
    
    tie(index1, index2) = index_pv(at);

    return data_pv[index1][index2];
  }


  size_t size() const
  {
    return size_pv;
  }

  void push_back (T t)
  {
    if (size_pv == 0)

      {
        if (!data_pv[index1_pv(size_pv)]) data_pv[0] = (T*) new char[sizeof(T)];
      }
    
    else if (index1_pv(size_pv-1) != index1_pv(size_pv))
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

  
};

}//namespace rpnx

#endif
