//
// Copyright 2018 Yoshinori Suzuki<wave.suzuki.z@gmail.com>
//
#pragma once

#include <atomic>
#include <cstddef>
#include <vector>

//
// atomic queue <pointer> class
//
template <class T>
class Queue
{
  std::vector<T*> buffer;
  std::atomic_int start{0};
  std::atomic_int end{0};

public:
  Queue(size_t sz)
  {
    buffer.resize(sz);
    std::fill(buffer.begin(), buffer.end(), nullptr);
  }

  //
  bool push(T* p)
  {
    int n, e = end;
    do
    {
      n = (e + 1) % buffer.size();
      if (n == start)
        return false;
    } while (end.compare_exchange_weak(e, n) == false);
    buffer[e] = p;
    return true;
  }

  //
  T* pop()
  {
    int s = start;
    if (s == end)
      return nullptr;
    int n = (s + 1) % buffer.size();
    if (start.compare_exchange_weak(s, n) == false)
      return nullptr;

    volatile auto p = buffer[s];
    while (p == nullptr)
      p = buffer[s];

    buffer[s] = nullptr;
    return p;
  }
  bool empty() const { return start == end; }
};

//
// atomic queue <number> class
//
template <class T>
class NQueue
{
  std::vector<T>  buffer;
  std::atomic_int start{0};
  std::atomic_int end{0};
  T               invalid_value;

public:
  NQueue(size_t sz, T invalid = 0)
  {
    invalid_value = invalid;
    buffer.resize(sz);
    std::fill(buffer.begin(), buffer.end(), invalid);
  }

  //
  bool push(T p)
  {
    int n, e = end;
    do
    {
      n = (e + 1) % buffer.size();
      if (n == start)
        return false;
    } while (end.compare_exchange_weak(e, n) == false);
    buffer[e] = p;
    return true;
  }

  //
  struct Result
  {
    T    value;
    bool ok;
    //
    operator bool() const { return ok; }
  };
  Result pop()
  {
    int s = start;
    if (s == end)
      return Result{invalid_value, false};
    int n = (s + 1) % buffer.size();
    if (start.compare_exchange_weak(s, n) == false)
      return Result{invalid_value, false};

    volatile auto p = buffer[s];
    while (p == invalid_value)
      p = buffer[s];

    buffer[s] = invalid_value;
    return Result{p, true};
  }
  bool empty() const { return start == end; }
};
