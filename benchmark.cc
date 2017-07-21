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
