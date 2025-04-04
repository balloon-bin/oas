# Building

To build oas in the default configuration you just need (gnu) make and a
sufficiently modern clang.

```
make
```

## Make targets

There are a number of make targets available to build various instrumented
builds that are used in validation, analysis and sanitizing. Some of these may
require extra dependencies.


 - `debug`: Creates the debug build in `build/debug`. This is the default target.
 - `all`: Builds all binary executable targets. These are
   `debug`, `release`, `msan`, `asan` and `afl`. All executables can be found
   in `build/` in a subdirectory matching their target names.
 - `release`: Creates the release build in `build/release`
 - `afl`: Creates a build with AFL++ instrumentation for fuzzing
 - `fuzz`: Starts the fuzzer with the instrumented afl executable
 - `asan`: builds with the address and undefined clang sanitizers
 - `msan`: builds with the memory clang sanitizer
 - `validate`: Builds `debug`, `msan`, and `asan` targets, then runs the
   validation script. This script executes the sanitizer targets and runs
   Valgrind on the debug target across multiple modes and test input files.

