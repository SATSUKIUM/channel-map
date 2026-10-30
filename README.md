# channel-map

A C++17 channel mapping library for realtime detector readout processing.
`chmap::ChannelMapDopeness` maps FEE addresses to detector identifiers and
optionally supports inverse lookup. Detector geometry and calibration objects
are owned by the map.

See the [Japanese documentation](docs/index.html) and
[dope_skeleton.cpp](dope_skeleton.cpp) for complete examples.

## Build and install

Requirements: a C++17 compiler, CMake 3.10 or later, and a build tool such as
Make or Ninja. ROOT is not required. Git is only needed if you choose to clone
the repository; source archives can be built without Git.

From the source directory:

```sh
cmake -S . -B .build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$HOME/.local/channel-map"
cmake --build .build --parallel
cmake --install .build
```

Use `-DCMAKE_BUILD_TYPE=Debug` for a debug build. The version is read from
`VERSION`. Both configurations build the library and current skeleton.

In the consuming project's `CMakeLists.txt`:

```cmake
find_package(ChannelMap REQUIRED)
target_link_libraries(YourProject PRIVATE ChannelMap::ChannelMap)
target_compile_features(YourProject PRIVATE cxx_std_17)
```

Configure the consumer with
`-DCMAKE_PREFIX_PATH="$HOME/.local/channel-map"`.

## CSV format

Use these nine columns in order:

```csv
fe.id,fe.channel,fe.data,detector.id,detector.plane,detector.segment,detector.channel,detector.readout,detector.data
0xc0a802a9,8,0,utof,0,0,0,left,0
0xc0a802a9,10,0,utof,0,0,0,right,0
```

FEE addresses use the lower two IP octets and an 8-bit channel number.
Detector names, plane names and readout names are indexed through dictionaries.
The segment number is 8-bit and detector channel number is 16-bit.
`fe.data` and `detector.data` are reserved and unused.

Keep the mapping one-to-one. Initialization reports duplicate entries but
continues loading. The all-zero FEE tuple is reserved for unmapped slots.
CSV parsing uses simple comma separation; quoted commas are not supported.
`mapdata_T103_20260404.csv` is the included nine-column map.

## Lookup

```cpp
#include <chmap/channel_map_dopeness.hpp>

void example() {
    auto& map = chmap::ChannelMapDopeness::get_instance();
    map.initialize("mapdata_T103_20260404.csv", true);

    if (const auto* det = map.findDET(2, 169, 8)) {
        const auto* fe = map.findFE(*det);
        // Use det and fe after checking for nullptr.
        (void)fe;
    }
}
```

Initialize once per process. `true` creates the inverse map; it defaults to
`false`. Checked lookups return `nullptr` for unmapped inputs. The retained
`getDopeKey_*` methods only check ranges and do not establish existence.

## Geometry and calibration

Pass ownership with `std::unique_ptr` to `setGeometry`, `setTdcCalibration`, or
`setDriftCalibration`, using either a `DETIdItem` or `FEAddrItem` as the target.
DET-based registration requires the inverse map. Read settings through
`det->getConfiguration()` and its const getters.

Repeated assignments, missing targets and null inputs return `false`.
Arguments are consumed even on failure. Accepted objects remain alive until
the map is destroyed; existing settings are never replaced.

## Compatibility

The older `ChannelMap`, `ChannelMapSimple`, and `ChannelMapDirect` APIs are no
longer shipped. Rebuild consumers against the matching headers and library.
