#include "ProfilerAPI.hpp"
#include "ProfilerNew.hpp"
#include <cassert>
struct Foo { int x; Foo(int v):x(v){} };

int main() {
  auto m1 = mp::api::getMetricsJson();
  assert(!m1.empty());

  // Asignación con metadatos (file/line/type)
  auto p = MP_NEW_FT(Foo, 42);
  auto m2 = mp::api::getMetricsJson();
  assert(!m2.empty());

  delete p;
  auto m3 = mp::api::getMetricsJson();
  assert(!m3.empty());
  return 0;
}
