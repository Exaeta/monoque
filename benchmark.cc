#include "monoque.hh"
#include <random>
#include <vector>
#include <iostream>
#include <iomanip>
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

  mt19937_64 r;

  size_t round_count = 300000000;
  size_t round_secondary = round_count*4;
  size_t runs = 3;

  vector<size_t> rds;
  for (size_t i = 0; i < round_count; i++)
    {
      rds.push_back(r() % round_count);
    }

  system_clock::time_point start_time;
  system_clock::time_point end_time;

  system_clock::time_point start_time_tt;
  system_clock::time_point end_time_tt;

  cout  << "For linear structures of size " << round_count << " and random access " << round_secondary << " times, discovered that: " << endl;

  double fast_push = std::numeric_limits<double>::max();
  double fast_access = std::numeric_limits<double>::max();
  cout << fixed;


  fast_push = std::numeric_limits<double>::max();
  fast_access = std::numeric_limits<double>::max();
  for (size_t r=0 ; r<runs ; ++r)
  {
    vector<size_t> m;
    start_time = system_clock::now();

    for (size_t i = 0; i < round_count; i++)
    {
      m.push_back(i);
    }
    end_time = system_clock::now();
    fast_push = std::min(fast_push, (double)(duration_cast<nanoseconds>(end_time-start_time).count()));
    cout << m.size() << ' ' << fast_push << endl;

    start_time = system_clock::now();
    size_t t = 0;
    for (size_t i = 0; i < round_secondary; i++)
    {
      t += m[rds[i % round_count]];
    }
    end_time = system_clock::now();
    fast_access = std::min(fast_access, (double)(duration_cast<nanoseconds>(end_time-start_time).count()));
    cout << t << ' ' << fast_access << endl;

  }
  cout << "vector<size_t> average push_back time: " << fast_push/round_count << " nanoseconds" << endl;
  cout << "vector<size_t> average random access time: " << fast_access/round_secondary << " nanoseconds" << endl;


  fast_push = std::numeric_limits<double>::max();
  fast_access = std::numeric_limits<double>::max();
  for (size_t r=0 ; r<runs ; ++r)
  {
    monoque<size_t> m;
    start_time = system_clock::now();

    for (size_t i = 0; i < round_count; i++)
    {
      m.push_back(i);
    }
    end_time = system_clock::now();
    fast_push = std::min(fast_push, (double)(duration_cast<nanoseconds>(end_time-start_time).count()));
    cout << m.size() << ' ' << fast_push << endl;

    start_time = system_clock::now();
    size_t t = 0;
    for (size_t i = 0; i < round_secondary; i++)
    {
      t += m[rds[i % round_count]];
    }
    end_time = system_clock::now();
    fast_access = std::min(fast_access, (double)(duration_cast<nanoseconds>(end_time-start_time).count()));
    cout << t << ' ' << fast_access << endl;

  }
  cout << "monoque<size_t> average push_back time: " << fast_push/round_count << " nanoseconds" << endl;
  cout << "monoque<size_t> average random access time: " << fast_access/round_secondary << " nanoseconds" << endl;


  fast_push = std::numeric_limits<double>::max();
  fast_access = std::numeric_limits<double>::max();
  for (size_t r=0 ; r<runs ; ++r)
  {
    deque<size_t> m;

    start_time = system_clock::now();
    for (size_t  i = 0; i < round_count; i++)
    {
      m.push_back(i);
    }
    end_time = system_clock::now();
    fast_push = std::min(fast_push, (double)(duration_cast<nanoseconds>(end_time-start_time).count()));
    cout << m.size() << ' ' << fast_push << endl;

    start_time = system_clock::now();
    size_t t = 0;
    for (size_t i = 0; i < round_secondary; i++)
    {
      t += m[rds[i % round_count]];
    }
    end_time = system_clock::now();
    fast_access = std::min(fast_access, (double)(duration_cast<nanoseconds>(end_time-start_time).count()));
    cout << t << ' ' << fast_access << endl;

  }
  cout << "deque<size_t> average push_back time: " << fast_push/round_count <<  " nanoseconds" << endl;
  cout << "deque<size_t> average random access time: " << fast_access/round_secondary << " nanoseconds" << endl;
}
