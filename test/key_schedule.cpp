#include "test_vectors.h"
#include <doctest/doctest.h>
#include <mls/state.h>
#include <mls_vectors/mls_vectors.h>

using namespace mls;
using namespace mls_vectors;

TEST_CASE("Encryption Keys Interop")
{
  for (auto suite : all_supported_suites) {
    const auto tv = EncryptionKeyTestVector(suite, 15, 10);
    REQUIRE(tv.verify() == std::nullopt);
  }
}

TEST_CASE("Key Schedule Interop")
{
  for (auto suite : all_supported_suites) {
    auto tv = KeyScheduleTestVector(suite, 15);
    REQUIRE(tv.verify() == std::nullopt);
  }
}
