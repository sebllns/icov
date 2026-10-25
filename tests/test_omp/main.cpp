#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <omp.h>
#include <unistd.h>

double f(double x) { return std::sqrt(3.14 * x + 1) / (x + 1); }

int main(int argc, char *argv[]) {

  const int count = 1000;
  double a[count];
  double b[count];

  {
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < count; ++i) {
      a[i] = f(i);
      usleep(100);
    }
    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << elapsed_seconds.count()
              << std::endl; // C++20: operator<< chrono::duration
  }

  {
    const auto start = std::chrono::steady_clock::now();
#pragma omp parallel for
    for (int i = 0; i < count; ++i) {
      b[i] = f(i);
      assert(b[i] == a[i]);
      usleep(100);
    }
    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << elapsed_seconds.count()
              << std::endl; // C++20: operator<< chrono::duration
  }

  return 0;
}
