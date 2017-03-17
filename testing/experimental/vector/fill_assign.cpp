#include <iostream>
#include <cassert>
#include <algorithm>
#include <agency/experimental/vector.hpp>

void test_reallocating_fill_assign()
{
  using namespace agency::experimental;

  {
    // test fill assign into empty vector
    
    vector<int> v;

    size_t num_elements_to_assign = 5;
    int assign_me = 7;

    v.assign(num_elements_to_assign, assign_me);

    assert(v.size() == num_elements_to_assign);
    assert(std::count(v.begin(), v.end(), assign_me) == static_cast<int>(v.size()));
  }

  {
    // test fill assign into small vector

    vector<int> v(3);

    size_t num_elements_to_assign = 5;
    int assign_me = 7;

    v.assign(num_elements_to_assign, assign_me);

    assert(v.size() == num_elements_to_assign);
    assert(std::count(v.begin(), v.end(), assign_me) == static_cast<int>(v.size()));
  }
}

void test_nonreallocating_fill_assign()
{
  using namespace agency::experimental;

  {
    // test range assign into empty vector with capacity

    vector<int> v;
    
    size_t num_elements_to_assign = 5;
    v.reserve(5);

    int assign_me = 7;

    v.assign(num_elements_to_assign, assign_me);

    assert(v.size() == num_elements_to_assign);
    assert(std::count(v.begin(), v.end(), assign_me) == static_cast<int>(v.size()));
  }

  {
    // test range assign into small vector with capacity

    vector<int> v(3);
    
    size_t num_elements_to_assign = 5;
    v.reserve(5);

    int assign_me = 7;

    v.assign(num_elements_to_assign, assign_me);

    assert(v.size() == num_elements_to_assign);
    assert(std::count(v.begin(), v.end(), assign_me) == static_cast<int>(v.size()));
  }

  {
    // test range assign into large vector
    size_t num_elements_to_assign = 5;
    
    vector<int> v(2 * num_elements_to_assign);

    int assign_me = 7;

    v.assign(num_elements_to_assign, assign_me);

    assert(v.size() == num_elements_to_assign);
    assert(std::count(v.begin(), v.end(), assign_me) == static_cast<int>(v.size()));
  }
}

int main()
{
  test_reallocating_fill_assign();
  test_nonreallocating_fill_assign();

  std::cout << "OK" << std::endl;

  return 0;
}

