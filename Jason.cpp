
#include <chrono>

#include "Jason.h"

int64_t arangodb::jason::CurrentUTCDateValue () {
  return static_cast<int64_t>(std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
}
