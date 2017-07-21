/*
"Monoque Data Structure Benchmark" http://rpnx.net/monoque.pdf
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

#include "monoque.hh"
#include <random>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <deque>
#include <atomic>
#include <thread>

int main()
{
  using namespace std;
  using namespace std::chrono;
  using namespace rpnx;

  std::ofstream o1("monoque_benchmark.txt");
  //  std::ofstream o1r("monoque::operator[]const.txt");
  std::ofstream o2("vector_benchmark.txt");
  // std::ofstream o2r("vector::operator[]const.txt");
  std::ofstream o3("deque_benchmark.txt");

  mt19937_64 r;
 
  size_t round_count = 300000000;
  size_t round_secondary = round_count*4;

  /*
  {
    monoque<int> m;
    for (int i = 0; i < round_count; i++)
    {
      m.push_back(i);
    }
  }

  {
    vector<int> m;
    for (int i = 0; i < round_count; i++)
    {
      m.push_back(i);
    }
  }
  */
  
  vector<size_t> rds;

  for (int i = 0; i < round_count; i++)
    {
      rds.push_back(r() % round_count);
    }
  
  system_clock::time_point start_time;
  system_clock::time_point end_time;

  system_clock::time_point start_time_tt;
  system_clock::time_point end_time_tt;

  cout  << "For linear structures of size " << round_count << " and random access " << round_secondary << " times, discovered that: " << endl;
 
  {
    vector<int> m;
    start_time = system_clock::now();

    volatile int tester;
  
    for (size_t i = 0; i < round_count; i++)
    {
      tester = i;      
      m.push_back(int(tester));    
    }
    end_time = system_clock::now();
    cout << "vector<int> average push_back time: " << (double)(duration_cast<nanoseconds>(end_time-start_time).count())/round_count << " nanoseconds" << endl;
    this_thread::yield();
    
    start_time = system_clock::now();
    for (size_t i = 0; i < round_secondary; i++)
    {     
      tester = m[rds[i % round_count]];
    }
    end_time = system_clock::now();
    cout << "vector<int> average random access time: " << (double)(duration_cast<nanoseconds>(end_time-start_time).count())/round_secondary << " nanoseconds" << endl;
    
  }

  {
    monoque<int> m;
    start_time = system_clock::now();

    volatile int tester;
  
    for (size_t i = 0; i < round_count; i++)
    {
      tester = i;      
      m.push_back(int(tester));    
    }
    end_time = system_clock::now();
    cout << "monoque<int> average push_back time: " << (double)(duration_cast<nanoseconds>(end_time-start_time).count())/round_count << " nanoseconds" << endl;
    this_thread::yield();
    
    start_time = system_clock::now();
    for (size_t i = 0; i < round_secondary; i++)
    {     
      tester = m[rds[i % round_count]];
    }
    end_time = system_clock::now();
    cout << "monoque<int> average random access time: " << (double)(duration_cast<nanoseconds>(end_time-start_time).count())/round_secondary << " nanoseconds" << endl;
    
  }



 /*
  {


    vector<int> m;
    for (int  i = 0; i < round_count; i++)
    {
      start_time = system_clock::now();
      std::atomic_signal_fence(memory_order_seq_cst);
      m.push_back(i);
      std::atomic_signal_fence(memory_order_seq_cst);
      end_time = system_clock::now();
      o2 << duration_cast<nanoseconds>(end_time-start_time).count() << endl;
      this_thread::yield();
    }


    start_time = system_clock::now();
    volatile int tester;
    for (int i = 0; i < round_count; i++)
    {
      std::atomic_signal_fence(memory_order_seq_cst);
      tester = m[rds[i]];      
      std::atomic_signal_fence(memory_order_seq_cst);
    }
    end_time = system_clock::now();
    //duration_cast<nanoseconds>(end_time-start_time).count())/round_count << endl;

  }
 */
  


  {
   
    deque<int> m;
    volatile int tester;

    start_time = system_clock::now();
    for (size_t  i = 0; i < round_count; i++)
    {
      tester = i;
      m.push_back(int(tester));
    }
    end_time = system_clock::now();
    cout << "deque<int> average push_back time: " << double(duration_cast<nanoseconds>(end_time-start_time).count())/round_count <<  " nanoseconds" << endl;
    this_thread::yield();

    start_time = system_clock::now();
    
    for (size_t i = 0; i < round_secondary; i++)
    {
      tester = m[rds[i % round_count]];
    }

    end_time = system_clock::now();
    cout << "deque<int> average random access time: " << (double)(duration_cast<nanoseconds>(end_time-start_time).count())/round_secondary << " nanoseconds" << endl;

  }

  return 0;

}
