# CHERI CSA

This is the fork of [The CHERI LLVM Compiler Infrastructure](https://github.com/CTSRD-CHERI/llvm-project) for CHERI-related Clang Static Analyzer development.

CSA performs inter-procedural path-sensitive analysis in the boundary of one translation unit (via inlining). Cross Translation Unit analysis exists, but I'm unsure about its status[^1].

[^1]:https://clang.llvm.org/docs/analyzer/user-docs/CrossTranslationUnit.html


## CHERI-related checkers

### ProvenanceSourceChecker

* Tracks integers and pointers stored as `(u)intptr_t` type
* Fires a warning for `(u)intptr_t` binary operations with ambiguous provenance source (same as clang) and tells which side carries (or not) the provenance along the path
* Fires a warning when the `(u)intptr_t` value obtained from the ambiguous-provenance-operation is cast to pointer type
* Fires a warning when `NULL`-derived `(u)intptr_t` capability is cast to pointer type

### CapabilityCopyChecker

Detects tag-stripping loads and stores that may be used to copy or swap capabilities

```c
void memcpy_impl(void* src0, void *dst0, size_t len) {
  char *src = src0;
  char *dst = dst0;

  while (len--)
    *dst++ = *src++; // Tag-stripping store of a capability
}
```


## Build

Use fork of [cheribuild](https://github.ckm/rems-project/cheribuild/commits/use-csa) to build from the source code:

```
cheribuild.py cheri-csa
```

## Usage

### Single compilation

1. Compile with clang from cheri-csa build
2. Add ``--analyze -Xanalyzer -analyzer-checker=core,alpha.cheri.ProvenanceSourceChecker,alpha.cheri.CapabilityCopyChecker[,OTHER_CHECKERS]`` to clang options.

### Analysing projects with scan-build

#### Using scan-build directly

```
$ ~/cheri/output/cheri-csa/bin/scan-build --keep-cc \
    -enable-checker alpha.cheri.ProvenanceSourceChecker,alpha.cheri.CapabilityCopyChecker \
    --use-cc ~/cheri/output/sdk/bin/clang \
    --use-c++ ~/cheri/output/sdk/bin/clang++ \
    BUILD_COMMAND
$ ~/cheri/output/cheri-csa/bin/scan-view /tmp/scan-build-<timestamp>
```

The idea is to trick the build system into calling the ccc-analyser wrapper instead of the original compiler. ccc-analyser, in turn, invokes the original compiler (provided by ``--use-cc``) and its own clang for static analysis, passing all the compiler options provided by the build system to both[^2].

[^2]:https://clang-analyzer.llvm.org/scan-build.html

Therefore BUILD_COMMAND should either
* use CC and CXX variables for obtaining the compiler path (scan-build will set CC and CXX to ccc-analyser and cxx-analyser, respectively),
* or directly invoke ccc/cxx-analyser instead of cheri-clang.
    
#### Analysing with cheribuild

``use-csa`` flag was added to the fork of cheribuild [here](https://github.ckm/rems-project/cheribuild/commits/use-csa) to support analysing projects that can be built with cheribuild.

```
$ cheribuild.py <project>-riscv64-purecap --<project>/use-csa --skip-install --clean
$ ~/cheri/output/cheri-csa/bin/scan-view /tmp/scan-build-<timestamp>
```


##### To add ``use-csa`` option to the project:

1. Audit all usages of `self.CC` within the project file, and consider replacing them with `self.cc_wrapper` (see [above](#using-scan_build-directly)).
2. Override the ``can_build_with_csa`` classmethod of the project class to return `True`.
