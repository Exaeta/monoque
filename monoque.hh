/*
"Monoque Data Structure Implementation"
Copyright (c) 2017 Ryan P. Nicholl <r.p.nicholl@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms are permitted
provided that the above copyright notice and this paragraph are
duplicated in all such forms and that any documentation,
advertising materials, and other materials related to such
distribution and use acknowledge that the software was developed
by the <organization>. The name of the
<organization> may not be used to endorse or promote products derived
from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

// 2017 by Ryan P. Nicholl <r.p.nicholl@gmail.com>

#ifndef RPNX_MONOQUE_HH
#define RPNX_MONOQUE_HH

#include <array>
#include <tuple>

/*
  Like vector, but non-contiguous and has worst-case O(1) push_back and worst-case O(1) indexing.
  This is just a simple proof of concept. It does not have most of the required STL elements and 
  is not exception safe, and does not have a way to delete the values after they are no longer
  needed. Also, the indexing was simplified significantly from the original design to make it
  simpler to illustrate.

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
    return n == 0 ? 0 : i + 1;
#else
    int i = 0; while ((1 << i) <= n) i++;
#endif
    return i;
  }

  static inline size_t sizeat_pv(size_t at)
  {
    return 1 << index1_pv(at);
  }
 
  static inline size_t index2_pv(size_t at)
  {
    return at & ((1<<index1_pv(at))-1);
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
