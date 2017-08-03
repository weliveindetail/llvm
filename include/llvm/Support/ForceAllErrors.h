#ifndef LLVM_SUPPORT_FORCEALLERRORS_H
#define LLVM_SUPPORT_FORCEALLERRORS_H

#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"

#include <cassert>
#include <memory>
#include <system_error>

namespace llvm {

class ErrorInfoBase;
class ForceAllErrorsInScope;

class ForceAllErrors {
public:
  ForceAllErrors() = default;

  void BeginCounting(ForceAllErrorsInScope *Caller) {
    assert(Guard == nullptr);
    Guard = Caller;
    InstanceCount = 0;
    InstanceToBreak = 0;
  }

  void BeginBreakInstance(int ForceErrorNumber, ForceAllErrorsInScope *Caller) {
    assert(Guard == nullptr);
    Guard = Caller;
    InstanceCount = 0;
    InstanceToBreak = ForceErrorNumber;
  }

  void End() {
    assert(Guard != nullptr);
    Guard = nullptr;
  }

  int getCount() const { return InstanceCount; }

  static ForceAllErrors &getInstance() { return *GlobalInstance; }

  static bool TurnInstanceIntoError() {
    ForceAllErrors &FAE = getInstance();

    if (!FAE.Guard)
      return false;

    return ++FAE.InstanceCount == FAE.InstanceToBreak;
  }

  static std::unique_ptr<ErrorInfoBase> mockError();
  static std::error_code mockErrorCode();

private:
  int InstanceCount = 0;
  int InstanceToBreak = 0;
  ForceAllErrorsInScope *Guard = nullptr;
  static ManagedStatic<ForceAllErrors> GlobalInstance;
};

class ForceAllErrorsInScope {
  enum ModeT { Disabled = 0, Count, Break };

public:
  ForceAllErrorsInScope(int ForceErrorNumber, raw_ostream &OS = nulls())
      : Mode(getModeFromClArg(ForceErrorNumber)), OutStream(OS) {
    ForceAllErrors &FAE = ForceAllErrors::getInstance();

    switch (Mode) {
    case Count:
      FAE.BeginCounting(this);
      OutStream << "Start counting mutation points\n";
      break;
    case Break:
      FAE.BeginBreakInstance(ForceErrorNumber, this);
      OutStream << "Wait for instance to break #" << ForceErrorNumber << "\n";
      break;
    case Disabled:
      break;
    }
  }

  ~ForceAllErrorsInScope() {
    ForceAllErrors &FAE = ForceAllErrors::getInstance();

    if (Mode == Count) {
      OutStream << "Found " << FAE.getCount() << " mutation points\n";
    }

    FAE.End();
  }

private:
  ModeT Mode;
  raw_ostream &OutStream;

  ModeT getModeFromClArg(int ForceErrorNumber) {
    if (ForceErrorNumber < 0)
      return Disabled;

    if (ForceErrorNumber > 0)
      return Break;

    return Count;
  }
};

} // namespace llvm

#endif // LLVM_SUPPORT_FORCEALLERRORS_H
