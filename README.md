# Gurux.Compression.cpp

V.44 compression and decompression for C++98 and newer, compatible with
Gurux.Compression.Net. Requires CMake 3.10 or newer and a C++ compiler.

## Getting started

Place this repository in your application's `Gurux.Compression.cpp` directory.
Add the library to your application's `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(CompressionExample LANGUAGES CXX)

add_subdirectory(Gurux.Compression.cpp)
add_executable(example main.cpp)
target_link_libraries(example PRIVATE gurux_compression)
```

Build your application from its root directory:

```sh
cmake -S . -B build
cmake --build build --config Release
```

## Example

Save this as `main.cpp`:

```cpp
#include "GXV44Encoder.h"
#include "GXV44Decoder.h"
#include <cstdio>

int main()
{
    const char text[] = "Hello, hello, hello!";
    CGXByteBuffer input, compressed, restored;
    CGXV44Encoder encoder;
    CGXV44Decoder decoder;

    int result = input.Set(text, sizeof(text) - 1);
    if (result == 0)
        result = encoder.Compress(input, compressed);
    if (result == 0)
        result = decoder.Decompress(compressed, restored);
    if (result != 0)
    {
        std::fprintf(stderr, "Compression error: %d\n", result);
        return 1;
    }

    std::fwrite(restored.GetData(), 1, restored.GetSize(), stdout);
    std::putchar('\n');
    return 0;
}
```

Encoder and decoder must use the same options. The example uses defaults.
Input and output buffers must be separate objects. A return value of zero means success.

## License

GPL-2.0-only or a separate commercial license from Gurux Ltd.
See [LICENSE](LICENSE) and [LICENSES](LICENSES).
