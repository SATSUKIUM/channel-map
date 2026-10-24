# Checked lookup validation

From the repository root (C++17 compiler required):

```sh
c++ -std=c++17 -O2 -Iinclude tests/checked_lookup.cpp src/*.cpp -o /tmp/channel-map-test
/tmp/channel-map-test /tmp/channel-map-test.csv both
/tmp/channel-map-test /tmp/channel-map-test.csv forward
/tmp/channel-map-test /tmp/channel-map-test.csv zero

c++ -std=c++17 -O3 -Iinclude tests/lookup_benchmark.cpp src/*.cpp -o /tmp/channel-map-benchmark
/tmp/channel-map-benchmark mapdata_T103_20260404.csv
```

Each functional case runs in a separate process because reinitialization is not supported.
The test CSV path is overwritten. Assertions must remain enabled.

`findDET` and `findFE` return pointers to existing items or nullptr. Legacy
`getDopeKey_*` still checks ranges only. Detector name index 0 is reserved;
detector names now use indices 1–255. Do not persist dictionary indices.
The all-zero FE tuple (ip3rd=0, ip4th=0, ch=0) is reserved and rejected on input.
Zero in an individual FE field remains valid. Item layouts are unchanged.
Both lookup APIs are safe before initialization; inverse lookup returns nullptr
when the inverse map has not been created. Returned pointers must not outlive
map storage. Concurrent mutation is not supported.

The benchmark alternates the legacy key+getter and checked pointer API, consumes
the same channel numbers, and uses fixed random queries drawn from registered
channels with IP third octet 2. It does not measure misses, cold-cache behavior,
inverse lookup, or application throughput. Build without LTO to retain separate
translation-unit call overhead; results depend on compiler and hardware.

## Local validation (2026-10-25)

macOS, clang++: all three functional cases and the skeleton with
mapdata_T103_20260404.csv passed. AddressSanitizer/UndefinedBehaviorSanitizer
combined executable did not reach main in this environment and was interrupted;
no sanitizer pass is claimed. Full CMake/ROOT installation was not exercised.

Registered-channel benchmark, -O3, separate translation units, three rounds:
legacy 3.52–3.72 ns/lookup, checked 1.96–2.06 ns/lookup, matching checksums.
These timings are indicative only and do not establish performance on Linux/DAQ.
Rebuild the library and consumers together: dictionary internals and detector
name index assignments have changed. The HTML under docs remains the explicitly
dated snapshot of the earlier commit, not documentation of this new API.
