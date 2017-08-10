# llvm-ForceAllErrors

LLVM fork with built-in support for error path checks of llvm::Expected & llvm::ErrorOr.
Instrumentation of the `llvm-ar` tool illustrates possible usage. Tested on Mac OSX 10.12 only.
Please find a detailed description of the approach [here](https://github.com/weliveindetail/ForceAllErrors-in-LLVM).

## Build

```
~/Develop $ cmake --version
cmake version 3.8.2

~/Develop $ ninja --version
1.7.2

~/Develop $ clang --version
clang version 4.0.1 (tags/RELEASE_401/final)
Target: x86_64-apple-darwin16.7.0
Thread model: posix
InstalledDir: /usr/local/bin

~/Develop $ git clone https://github.com/weliveindetail/llvm-ForceAllErrors.git llvm
~/Develop $ cd llvm
~/Develop/llvm $ git checkout -b ForceAllErrors
~/Develop/llvm $ mkdir ../llvm-build
~/Develop/llvm $ cd ../llvm-build
~/Develop/llvm-build $ cmake -G Ninja -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_BUILD_TYPE=Debug -DCMAKE_OSX_SYSROOT=macosx10.12 -DLLVM_OPTIMIZED_TABLEGEN=ON -DLLVM_TARGETS_TO_BUILD=host -DLLVM_USE_SPLIT_DWARF=ON -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_DOCS=OFF ../llvm
~/Develop/llvm-build $ ninja llvm-ar
```

In order to build with Sanitizer like Address & Undefined Behavior, add configuration flag `-DLLVM_USE_SANITIZER="Address;Undefined"`.

## Run

```
~/Develop/llvm-build $ # count mutation points
~/Develop/llvm-build $ ./bin/llvm-ar -debug -debug-only=ForceAllErrors -force-error=0 t /usr/local/lib/libc++.a 2>&1 1>/dev/null
267

~/Develop/llvm-build $ # turn 6th instance into an error
~/Develop/llvm-build $ ./bin/llvm-ar -debug -debug-only=ForceAllErrors -force-error=6 t /usr/local/lib/libc++.a 2>&1 1>/dev/null
Aim to break instance #6
0  libLLVMSupport.dylib 0x0000000104c425df llvm::sys::PrintStackTrace(llvm::raw_ostream&, int) + 63
1  libLLVMSupport.dylib 0x0000000104b04238 llvm::ForceAllErrors::TurnInstanceIntoError() + 120
2  libLLVMObject.dylib  0x0000000104767993 llvm::Expected<llvm::StringRef>::Expected<llvm::StringRef>(llvm::StringRef&&, std::__1::enable_if<std::is_convertible<llvm::StringRef, llvm::StringRef>::value, void>::type*) + 51
3  libLLVMObject.dylib  0x0000000104751cd5 llvm::Expected<llvm::StringRef>::Expected<llvm::StringRef>(llvm::StringRef&&, std::__1::enable_if<std::is_convertible<llvm::StringRef, llvm::StringRef>::value, void>::type*) + 37
4  libLLVMObject.dylib  0x0000000104751c22 llvm::object::ArchiveMemberHeader::getRawName() const + 1058
5  libLLVMObject.dylib  0x00000001047562e0 llvm::object::Archive::Child::getRawName() const + 32
./bin/llvm-ar: error loading '/usr/local/lib/libc++.a': Bad file descriptor!: Bad file descriptor.
```

## Automation

```
~/Develop/llvm-build $ cat ForceAllErrors-llvm-ar.sh 
#!/bin/sh

MutationPoints=$(./bin/llvm-ar -debug -debug-only=ForceAllErrors -force-error=0 t /usr/local/lib/libc++.a 2>&1 1>/dev/null)
echo "MutationPoints: $MutationPoints"

p=1
while [ $p -le 267 ]
do
  echo "\nRunning: ./bin/llvm-ar -debug -debug-only=ForceAllErrors -force-error=$p t /usr/local/lib/libc++.a 2>&1 1>/dev/null"
  ./bin/llvm-ar -debug -debug-only=ForceAllErrors -force-error=$p t /usr/local/lib/libc++.a 2>&1 1>/dev/null
  p=`expr $p + 1`
done

~/Develop/llvm-build $ sh ForceAllErrors-llvm-ar.sh
MutationPoints: 267

Running: ./bin/llvm-ar -debug -debug-only=ForceAllErrors -force-error=1 t /usr/local/lib/libc++.a 2>&1 1>/dev/null
Aim to break instance #1
0  libLLVMSupport.dylib 0x00000001109455df llvm::sys::PrintStackTrace(llvm::raw_ostream&, int) + 63
1  libLLVMSupport.dylib 0x0000000110807238 llvm::ForceAllErrors::TurnInstanceIntoError() + 120
2  libLLVMSupport.dylib 0x00000001108315fb llvm::ErrorOr<std::__1::unique_ptr<llvm::MemoryBuffer, std::__1::default_delete<llvm::MemoryBuffer> > >::ErrorOr<std::__1::unique_ptr<llvm::MemoryBuffer, std::__1::default_delete<llvm::MemoryBuffer> > >(std::__1::unique_ptr<llvm::MemoryBuffer, std::__1::default_delete<llvm::MemoryBuffer> >&&, std::__1::enable_if<std::is_convertible<std::__1::unique_ptr<llvm::MemoryBuffer, std::__1::default_delete<llvm::MemoryBuffer> >, std::__1::unique_ptr<llvm::MemoryBuffer, std::__1::default_delete<llvm::MemoryBuffer> > >::value, void>::type*) + 59
3  libLLVMSupport.dylib 0x0000000110831215 llvm::ErrorOr<std::__1::unique_ptr<llvm::MemoryBuffer, std::__1::default_delete<llvm::MemoryBuffer> > >::ErrorOr<std::__1::unique_ptr<llvm::MemoryBuffer, std::__1::default_delete<llvm::MemoryBuffer> > >(std::__1::unique_ptr<llvm::MemoryBuffer, std::__1::default_delete<llvm::MemoryBuffer> >&&, std::__1::enable_if<std::is_convertible<std::__1::unique_ptr<llvm::MemoryBuffer, std::__1::default_delete<llvm::MemoryBuffer> >, std::__1::unique_ptr<llvm::MemoryBuffer, std::__1::default_delete<llvm::MemoryBuffer> > >::value, void>::type*) + 37
4  libLLVMSupport.dylib 0x0000000110830315 getOpenFileImpl(int, llvm::Twine const&, unsigned long long, unsigned long long, long long, bool, bool) + 965
5  libLLVMSupport.dylib 0x000000011082fe83 getFileAux(llvm::Twine const&, long long, unsigned long long, unsigned long long, bool, bool) + 243
./bin/llvm-ar: error opening '/usr/local/lib/libc++.a': Unknown error: -1!.

Running: ./bin/llvm-ar -debug -debug-only=ForceAllErrors -force-error=2 t /usr/local/lib/libc++.a 2>&1 1>/dev/null
Aim to break instance #2
0  libLLVMSupport.dylib 0x0000000107e475df llvm::sys::PrintStackTrace(llvm::raw_ostream&, int) + 63
1  libLLVMSupport.dylib 0x0000000107d09238 llvm::ForceAllErrors::TurnInstanceIntoError() + 120
2  libLLVMObject.dylib  0x0000000107969993 llvm::Expected<llvm::StringRef>::Expected<llvm::StringRef>(llvm::StringRef&&, std::__1::enable_if<std::is_convertible<llvm::StringRef, llvm::StringRef>::value, void>::type*) + 51
3  libLLVMObject.dylib  0x0000000107953cd5 llvm::Expected<llvm::StringRef>::Expected<llvm::StringRef>(llvm::StringRef&&, std::__1::enable_if<std::is_convertible<llvm::StringRef, llvm::StringRef>::value, void>::type*) + 37
4  libLLVMObject.dylib  0x0000000107953c22 llvm::object::ArchiveMemberHeader::getRawName() const + 1058
5  libLLVMObject.dylib  0x0000000107957572 llvm::object::Archive::Child::isThinMember() const + 82
./bin/llvm-ar: error loading '/usr/local/lib/libc++.a': Bad file descriptor!: Bad file descriptor.

...

Running: ./bin/llvm-ar -debug -debug-only=ForceAllErrors -force-error=267 t /usr/local/lib/libc++.a 2>&1 1>/dev/null
Aim to break instance #267
0  libLLVMSupport.dylib 0x00000001083625df llvm::sys::PrintStackTrace(llvm::raw_ostream&, int) + 63
1  libLLVMSupport.dylib 0x0000000108224238 llvm::ForceAllErrors::TurnInstanceIntoError() + 120
2  libLLVMObject.dylib  0x0000000107e8e953 llvm::Expected<llvm::object::Archive::Child>::Expected<llvm::object::Archive::Child>(llvm::object::Archive::Child&&, std::__1::enable_if<std::is_convertible<llvm::object::Archive::Child, llvm::object::Archive::Child>::value, void>::type*) + 51
3  libLLVMObject.dylib  0x0000000107e7c0e5 llvm::Expected<llvm::object::Archive::Child>::Expected<llvm::object::Archive::Child>(llvm::object::Archive::Child&&, std::__1::enable_if<std::is_convertible<llvm::object::Archive::Child, llvm::object::Archive::Child>::value, void>::type*) + 37
4  libLLVMObject.dylib  0x0000000107e7bcab llvm::object::Archive::Child::getNext() const + 267
5  llvm-ar              0x000000010646c707 llvm::object::Archive::child_iterator::operator++() + 167
./bin/llvm-ar: Mocked Error.
```
