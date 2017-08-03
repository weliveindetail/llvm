#include "llvm/Support/ForceAllErrors.h"

#include "llvm/Support/Error.h"

namespace llvm {
ManagedStatic<ForceAllErrors> ForceAllErrors::GlobalInstance;

std::unique_ptr<ErrorInfoBase> ForceAllErrors::mockError() {
  return make_unique<llvm::StringError>(
      "Mocked Error", std::error_code(9, std::system_category()));
}

std::error_code ForceAllErrors::mockErrorCode() {
  return std::error_code(-1, std::generic_category());
}
}
