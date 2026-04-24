#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "interface/ComInterface.hpp"

namespace eduart {
namespace sensorring {
namespace com {

struct RawCanFrame {
  std::uint32_t can_id{};
  std::vector<std::uint8_t> data;
  bool fd{ false };
};

struct CanFilter {
  std::uint32_t can_id{ 0U };
  std::uint32_t can_mask{ 0U };
};

class CanInterface : public ComInterface {
public:
  using CanFrameCallback = std::function<void(const RawCanFrame&)>;
  using ComInterface::ComInterface;

  virtual bool sendCanFrame(std::uint32_t can_id, const std::vector<std::uint8_t>& data, bool fd) = 0;

  Subscription subscribeCanFrames(CanFrameCallback callback, CanFilter filter = {});

protected:
  void dispatchCanFrame(const RawCanFrame& frame);

private:
  struct RawSubscriptionEntry {
    CanFrameCallback callback;
    CanFilter filter;
  };

  std::mutex _can_subscriber_mutex;
  std::unordered_map<SubscriberToken, RawSubscriptionEntry> _can_subscriptions;
};

} // namespace com
} // namespace sensorring
} // namespace eduart
