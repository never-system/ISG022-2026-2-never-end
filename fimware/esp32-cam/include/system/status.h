#ifndef STATUS_H
#define STATUS_H

namespace app {

enum class Status {
  kOk = 0,
  kInvalidArgument,
  kNotReady,
  kStorageError,
  kNetworkError,
  kTimeout,
  kCameraError,
  kParseError,
  kSendError
};

}  // namespace app

#endif
