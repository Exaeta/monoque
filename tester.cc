#include "monoque.hh"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <queue>
#include <vector>

class tester {
public:
  size_t volatile val;
  static size_t dval;

public:
  tester() {
    using namespace std;
    dval++;
    if (val == 0xfafa4343) {
      cout << "suspicious" << endl;
    }

    val = 0xfafa4343;
  }

  tester(tester const &other) : tester() {}

  tester(tester &&) : tester() {}

  ~tester() {
    using namespace std;
    if (val != 0xfafa4343) {
      cout << "test fail" << endl;
    }
    val = 0;
    dval--;
  }

  tester &operator=(tester const &) = default;
};

size_t tester::dval = 0;

int main() {
  using namespace std;
  using namespace rpnx;
#if false
  


  static_assert (is_same<decltype(monoque<int>::iterator().operator*()), int&>::value);
  
  using It = monoque<int>::iterator;
  using T = int;
  monoque<int>::iterator test1;
  // default constructor
  monoque<int>::iterator test2(static_cast<monoque<int>::iterator const&>(test1));
  // copy constructor
  monoque<int>::iterator test3(move(test2));
  // move construtor

 
  test3 =  test2;
  static_assert(is_same<decltype(test3 = test2), monoque<int>::iterator &>::value);
  // copy assignment

  test1 = move(test3);
  // move assignment

  std::swap(test1, test2);
  // swappable

  ++test1;
  static_assert(is_same<decltype(++test1), monoque<int>::iterator&>::value);

  static_assert(is_same<typename std::iterator_traits<It>::value_type, int>::value);

  static_assert (is_same<decltype(monoque<int>::iterator() + int()), monoque<int>::iterator>::value);
  static_assert (is_same<decltype(monoque<int>::iterator()[int()]), int&>::value);
  static_assert (is_same<decltype(monoque<int>::iterator().operator->()), int*>::value);
  
  {
    rpnx::monoque<tester> test;
    assert(test.size() == 0);
    test.push_back(tester());
    assert(test.size() == 1);
    for (size_t i = 0; i < 59; i++) test.push_back(tester());
    assert(test.size() == 60);
   }

  rpnx::monoque<int> test;
  test.push_back(6);
  test.push_back(5);
  test.push_back(7);

  assert(test.cbegin() != test.cend());
  for (auto &x: test) cout << "X=" << x << endl;
  auto it = test.begin();
  it++;
  ++it;
  it = test.end();

  it = test.begin();
  rpnx::monoque<int>::iterator &it2 = ++it;

  it = it;

  rpnx::monoque<int>::iterator it3 = it + 2;
  rpnx::monoque<int>::iterator it4 = it - 2;

  (void) (it3 < it4);
  (void) (it3 > it4);
  (void) (it3 == it4);
  (void) (it2 <= it4);
  (void) (it >= it2);
  (void) (it == it);
  (void) (it != it);

  assert(it4 + 4 == it3); 

  rpnx::monoque<int>::reference it5 = it[0];

  rpnx::monoque<int>::iterator const & it6 = it--;


  sort(test.begin(), test.end());
  vector<int> testvec ( test.begin(), test.end());
  assert((vector<int>(test.begin(), test.end()) == vector<int>{5, 6, 7}));
  assert(test.back() == 7);
#endif

  std::priority_queue<int, rpnx::monoque<int>> test_queue;

  test_queue.push(4);
  test_queue.push(3);
  test_queue.push(9);
  test_queue.push(5);

  cout << test_queue.top() << std::endl;
}
