#include "llvm/Support/ForceAllErrors.h"

#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "ForceAllErrors"

namespace llvm {
ManagedStatic<ForceAllErrors> ForceAllErrors::GlobalInstance;

std::unique_ptr<ErrorInfoBase> ForceAllErrors::mockError() {
  return make_unique<llvm::StringError>(
      "Mocked Error", std::error_code(9, std::system_category()));
}

std::error_code ForceAllErrors::mockErrorCode() {
  return std::error_code(-1, std::generic_category());
}

ForceAllErrorsInScope::ForceAllErrorsInScope(int ForceErrorNumber)
    : Mode(getModeFromClArg(ForceErrorNumber)) {
  ForceAllErrors &FAE = ForceAllErrors::getInstance();

  switch (Mode) {
  case Count:
    FAE.BeginCounting(this);
    DEBUG(dbgs() << "Start counting mutation points\n");
    break;
  case Break:
    FAE.BeginBreakInstance(ForceErrorNumber, this);
    DEBUG(dbgs() << "Break instance #" << ForceErrorNumber << "\n");
    break;
  case Disabled:
    break;
  }
}

ForceAllErrorsInScope::~ForceAllErrorsInScope() {
  ForceAllErrors &FAE = ForceAllErrors::getInstance();

  if (Mode == Count) {
    DEBUG(dbgs() << "Found " << FAE.getCount() << " mutation points\n");
  }

  FAE.End();
}
}
