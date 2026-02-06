// Unit tests for eduart::device::IDevice and capability infrastructure.

#include <algorithm>
#include <cctype>
#include <catch2/catch_all.hpp>
#include <future>
#include <string>
#include <typeindex>
#include <vector>

#include "sensorring/device/CapabilityException.hpp"
#include "sensorring/device/IDevice.hpp"

using eduart::device::CapabilityNotSupported;
using eduart::device::ICapability;
using eduart::device::ICapabilityAsync;
using eduart::device::IDevice;

// Capability types with struct Request/Response so overloads are distinct (cf. LightDevice, TemperatureSensorDevice).
struct AddCap {
  struct Request {
    int value;
  };
  struct Response {
    int value;
  };
};

struct AsyncCap {
  struct Request {
    int value;
  };
  struct Response {
    int value;
  };
};

struct SyncOnlyCap {
  struct Request {
    int value;
  };
  struct Response {
    int value;
  };
};

struct ConstOnlyCap {
  struct Request {
    int value;
  };
  struct Response {
    int value;
  };
};

struct UnregisteredCap {
  struct Request {
    int value;
  };
  struct Response {
    int value;
  };
};

// Capability with two inputs, one output
struct TwoInputOneOutputCap {
  struct Request {
    int a;
    int b;
  };
  struct Response {
    int result;
  };
};

// Capability with one input, two outputs
struct OneInputTwoOutputCap {
  struct Request {
    int value;
  };
  struct Response {
    int doubled;
    int squared;
  };
};

// Capability with two inputs, two outputs
struct TwoInputTwoOutputCap {
  struct Request {
    int x;
    int y;
  };
  struct Response {
    int sum;
    int product;
  };
};

// Capability with complex types (strings and vectors)
struct ProcessStringsCap {
  struct Request {
    std::vector<std::string> words;
    std::string prefix;
  };
  struct Response {
    std::vector<std::string> processed_words;
    std::string concatenated_result;
    int total_length;
  };
};

// Test device that exposes several capabilities via IDevice.
struct TestDevice : IDevice, ICapability<AddCap>, ICapabilityAsync<AsyncCap>, ICapability<SyncOnlyCap>, ICapability<ConstOnlyCap>, ICapability<TwoInputOneOutputCap>, ICapability<OneInputTwoOutputCap>, ICapability<TwoInputTwoOutputCap>, ICapability<ProcessStringsCap> {
  int add_base = 1;

  mutable int add_calls                  = 0;
  mutable int async_calls                = 0;
  mutable int sync_only_calls            = 0;
  mutable int const_only_calls           = 0;
  mutable int two_input_one_output_calls  = 0;
  mutable int one_input_two_output_calls  = 0;
  mutable int two_input_two_output_calls  = 0;
  mutable int process_strings_calls      = 0;

  TestDevice() {
    // Register capabilities with explicit names where useful.
    register_capability<AddCap>("add_capability");
    register_capability_async<AsyncCap>("async_capability");
    register_capability<SyncOnlyCap>("sync_only_capability");
    register_capability<ConstOnlyCap>("const_only_capability");
    register_capability<TwoInputOneOutputCap>("two_input_one_output");
    register_capability<OneInputTwoOutputCap>("one_input_two_output");
    register_capability<TwoInputTwoOutputCap>("two_input_two_output");
    register_capability<ProcessStringsCap>("process_strings");
  }

  // ICapability<AddCap> - non-const and const variants.
  AddCap::Response invoke(const AddCap::Request& req) override {
    ++add_calls;
    return AddCap::Response{ req.value + add_base };
  }

  AddCap::Response invoke(const AddCap::Request& req) const override {
    ++add_calls;
    return AddCap::Response{ req.value + add_base + 100 };
  }

  // ICapabilityAsync<AsyncCap> - non-const and const variants.
  std::future<AsyncCap::Response> invoke_async(const AsyncCap::Request& req) override {
    ++async_calls;
    return std::async(std::launch::deferred, [req]() {
      return AsyncCap::Response{ req.value * 2 };
    });
  }

  std::future<AsyncCap::Response> invoke_async(const AsyncCap::Request& req) const override {
    ++async_calls;
    return std::async(std::launch::deferred, [req]() {
      return AsyncCap::Response{ req.value * 3 };
    });
  }

  // ICapability<SyncOnlyCap> - only synchronous implementation.
  SyncOnlyCap::Response invoke(const SyncOnlyCap::Request& req) override {
    ++sync_only_calls;
    return SyncOnlyCap::Response{ -req.value };
  }

  // ICapability<ConstOnlyCap> - non-const delegates to const; tests prefer const path.
  ConstOnlyCap::Response invoke(const ConstOnlyCap::Request& req) override {
    return const_cast<const TestDevice*>(this)->invoke(req);
  }
  ConstOnlyCap::Response invoke(const ConstOnlyCap::Request& req) const override {
    ++const_only_calls;
    return ConstOnlyCap::Response{ req.value + 42 };
  }

  // ICapability<TwoInputOneOutputCap> - two inputs, one output (e.g., multiply)
  TwoInputOneOutputCap::Response invoke(const TwoInputOneOutputCap::Request& req) override {
    ++two_input_one_output_calls;
    return TwoInputOneOutputCap::Response{ req.a * req.b };
  }

  TwoInputOneOutputCap::Response invoke(const TwoInputOneOutputCap::Request& req) const override {
    ++two_input_one_output_calls;
    // Const version adds 10 to the result
    return TwoInputOneOutputCap::Response{ req.a * req.b + 10 };
  }

  // ICapability<OneInputTwoOutputCap> - one input, two outputs (e.g., compute doubled and squared)
  OneInputTwoOutputCap::Response invoke(const OneInputTwoOutputCap::Request& req) override {
    ++one_input_two_output_calls;
    return OneInputTwoOutputCap::Response{ req.value * 2, req.value * req.value };
  }

  OneInputTwoOutputCap::Response invoke(const OneInputTwoOutputCap::Request& req) const override {
    ++one_input_two_output_calls;
    // Const version adds 1 to both outputs
    return OneInputTwoOutputCap::Response{ req.value * 2 + 1, req.value * req.value + 1 };
  }

  // ICapability<TwoInputTwoOutputCap> - two inputs, two outputs (e.g., compute sum and product)
  TwoInputTwoOutputCap::Response invoke(const TwoInputTwoOutputCap::Request& req) override {
    ++two_input_two_output_calls;
    return TwoInputTwoOutputCap::Response{ req.x + req.y, req.x * req.y };
  }

  TwoInputTwoOutputCap::Response invoke(const TwoInputTwoOutputCap::Request& req) const override {
    ++two_input_two_output_calls;
    // Const version adds 5 to both outputs
    return TwoInputTwoOutputCap::Response{ req.x + req.y + 5, req.x * req.y + 5 };
  }

  // ICapability<ProcessStringsCap> - complex types (strings and vectors)
  ProcessStringsCap::Response invoke(const ProcessStringsCap::Request& req) override {
    ++process_strings_calls;
    std::vector<std::string> processed;
    std::string concatenated = req.prefix;
    int total_len            = 0;

    for (const auto& word : req.words) {
      std::string prefixed = req.prefix + word;
      processed.push_back(prefixed);
      concatenated += word + "_";
      total_len += static_cast<int>(word.length());
    }

    // Remove trailing underscore if any words were processed
    if (!req.words.empty() && !concatenated.empty()) {
      concatenated.pop_back();
    }

    return ProcessStringsCap::Response{ processed, concatenated, total_len };
  }

  ProcessStringsCap::Response invoke(const ProcessStringsCap::Request& req) const override {
    ++process_strings_calls;
    // Const version adds "CONST_" prefix and counts uppercase letters
    std::vector<std::string> processed;
    std::string concatenated = "CONST_" + req.prefix;
    int total_len            = 0;
    int uppercase_count      = 0;

    for (const auto& word : req.words) {
      std::string prefixed = "CONST_" + req.prefix + word;
      processed.push_back(prefixed);
      concatenated += word + "_";
      total_len += static_cast<int>(word.length());
      // Count uppercase letters in word
      for (char c : word) {
        if (std::isupper(static_cast<unsigned char>(c))) {
          ++uppercase_count;
        }
      }
    }

    if (!req.words.empty() && !concatenated.empty()) {
      concatenated.pop_back();
    }

    // Use uppercase_count as part of total_length in const version
    return ProcessStringsCap::Response{ processed, concatenated, total_len + uppercase_count };
  }

  // Helper to exercise re-registration / renaming logic.
  void rename_add_capability(const std::string& new_name) { register_capability<AddCap>(new_name); }
};

TEST_CASE("IDevice supports() and capabilities() reflect registered capabilities", "[IDevice]") {
  TestDevice dev;

  SECTION("supports<T> returns true for registered capabilities and false otherwise") {
    REQUIRE(dev.supports<AddCap>());
    REQUIRE(dev.supports<AsyncCap>());
    REQUIRE(dev.supports<SyncOnlyCap>());
    REQUIRE(dev.supports<ConstOnlyCap>());
    REQUIRE(dev.supports<TwoInputOneOutputCap>());
    REQUIRE(dev.supports<OneInputTwoOutputCap>());
    REQUIRE(dev.supports<TwoInputTwoOutputCap>());
    REQUIRE(dev.supports<ProcessStringsCap>());
    REQUIRE_FALSE(dev.supports<UnregisteredCap>());
  }

  SECTION("capabilities() lists all registered capabilities with correct type_index and name") {
    auto caps = dev.capabilities();
    REQUIRE(caps.size() == 8);

    auto find_cap = [&caps](std::type_index idx) -> const std::pair<std::type_index, std::string>* {
      auto it = std::find_if(caps.begin(), caps.end(), [idx](const auto& p) {
        return p.first == idx;
      });
      return it == caps.end() ? nullptr : &*it;
    };

    const auto* add_entry = find_cap(std::type_index(typeid(AddCap)));
    REQUIRE(add_entry != nullptr);
    REQUIRE(add_entry->second == "add_capability");

    const auto* async_entry = find_cap(std::type_index(typeid(AsyncCap)));
    REQUIRE(async_entry != nullptr);
    REQUIRE(async_entry->second == "async_capability");

    const auto* sync_only_entry = find_cap(std::type_index(typeid(SyncOnlyCap)));
    REQUIRE(sync_only_entry != nullptr);
    REQUIRE(sync_only_entry->second == "sync_only_capability");

    const auto* const_only_entry = find_cap(std::type_index(typeid(ConstOnlyCap)));
    REQUIRE(const_only_entry != nullptr);
    REQUIRE(const_only_entry->second == "const_only_capability");

    const auto* two_input_one_output_entry = find_cap(std::type_index(typeid(TwoInputOneOutputCap)));
    REQUIRE(two_input_one_output_entry != nullptr);
    REQUIRE(two_input_one_output_entry->second == "two_input_one_output");

    const auto* one_input_two_output_entry = find_cap(std::type_index(typeid(OneInputTwoOutputCap)));
    REQUIRE(one_input_two_output_entry != nullptr);
    REQUIRE(one_input_two_output_entry->second == "one_input_two_output");

    const auto* two_input_two_output_entry = find_cap(std::type_index(typeid(TwoInputTwoOutputCap)));
    REQUIRE(two_input_two_output_entry != nullptr);
    REQUIRE(two_input_two_output_entry->second == "two_input_two_output");

    const auto* process_strings_entry = find_cap(std::type_index(typeid(ProcessStringsCap)));
    REQUIRE(process_strings_entry != nullptr);
    REQUIRE(process_strings_entry->second == "process_strings");
  }

  SECTION("re-registering a capability updates its name") {
    dev.rename_add_capability("renamed_add_cap");
    auto caps = dev.capabilities();

    auto it = std::find_if(caps.begin(), caps.end(), [](const auto& p) {
      return p.first == std::type_index(typeid(AddCap));
    });
    REQUIRE(it != caps.end());
    REQUIRE(it->second == "renamed_add_cap");
  }
}

TEST_CASE("IDevice synchronous invoke returns optional result", "[IDevice]") {
  TestDevice dev;

  SECTION("non-const invoke for supported capability") {
    auto result = static_cast<IDevice&>(dev).invoke<AddCap>(AddCap::Request{ 5 });
    REQUIRE(result.has_value());
    REQUIRE(result->value == 6); // 5 + add_base(1)
  }

  SECTION("const invoke prefers const implementation when present") {
    const TestDevice& cdev = dev;
    auto result            = static_cast<const IDevice&>(cdev).invoke<AddCap>(AddCap::Request{ 5 });
    REQUIRE(result.has_value());
    REQUIRE(result->value == 106); // const impl adds extra 100
  }

  SECTION("invoke for unsupported capability returns empty optional") {
    auto result = static_cast<IDevice&>(dev).invoke<UnregisteredCap>(UnregisteredCap::Request{ 10 });
    REQUIRE_FALSE(result.has_value());
  }
}

TEST_CASE("IDevice asynchronous invoke returns optional future", "[IDevice]") {
  TestDevice dev;

  SECTION("non-const invoke_async uses async implementation when available") {
    auto fut_opt = static_cast<IDevice&>(dev).invoke_async<AsyncCap>(AsyncCap::Request{ 4 });
    REQUIRE(fut_opt.has_value());
    REQUIRE(fut_opt->get().value == 8); // 4 * 2
  }

  SECTION("const invoke_async prefers const async implementation when present") {
    const TestDevice& cdev = dev;
    auto fut_opt           = static_cast<const IDevice&>(cdev).invoke_async<AsyncCap>(AsyncCap::Request{ 4 });
    REQUIRE(fut_opt.has_value());
    REQUIRE(fut_opt->get().value == 12); // 4 * 3
  }

  SECTION("invoke_async wraps synchronous capability in std::async for non-const") {
    auto fut_opt = static_cast<IDevice&>(dev).invoke_async<SyncOnlyCap>(SyncOnlyCap::Request{ 7 });
    REQUIRE(fut_opt.has_value());
    REQUIRE(fut_opt->get().value == -7);
  }

  SECTION("const invoke_async wraps const synchronous capability in std::async when only const is available") {
    const TestDevice& cdev = dev;
    auto fut_opt           = static_cast<const IDevice&>(cdev).invoke_async<ConstOnlyCap>(ConstOnlyCap::Request{ 3 });
    REQUIRE(fut_opt.has_value());
    REQUIRE(fut_opt->get().value == 45); // 3 + 42
  }

  SECTION("invoke_async for unsupported capability returns empty optional") {
    auto fut_opt = static_cast<IDevice&>(dev).invoke_async<UnregisteredCap>(UnregisteredCap::Request{ 1 });
    REQUIRE_FALSE(fut_opt.has_value());
  }
}

TEST_CASE("IDevice try_invoke throws for unsupported capabilities", "[IDevice]") {
  TestDevice dev;

  SECTION("try_invoke returns response for supported capability") {
    auto resp = static_cast<IDevice&>(dev).try_invoke<AddCap>(AddCap::Request{ 2 });
    REQUIRE(resp.value == 3);
  }

  SECTION("const try_invoke also works for supported capability") {
    const TestDevice& cdev = dev;
    auto resp              = static_cast<const IDevice&>(cdev).try_invoke<AddCap>(AddCap::Request{ 2 });
    REQUIRE(resp.value == 103); // 2 + 1 + 100
  }

  SECTION("try_invoke throws CapabilityNotSupported for unsupported capability") {
    REQUIRE_THROWS_AS(static_cast<IDevice&>(dev).try_invoke<UnregisteredCap>(UnregisteredCap::Request{ 1 }), CapabilityNotSupported);

    try {
      (void)static_cast<IDevice&>(dev).try_invoke<UnregisteredCap>(UnregisteredCap::Request{ 1 });
      FAIL("Expected CapabilityNotSupported to be thrown");
    } catch (const CapabilityNotSupported& ex) {
      REQUIRE(ex.capability_type == std::type_index(typeid(UnregisteredCap)));
    }
  }
}

TEST_CASE("IDevice try_invoke_async throws for unsupported capabilities", "[IDevice]") {
  TestDevice dev;

  SECTION("try_invoke_async returns future for capability with async implementation") {
    auto fut = static_cast<IDevice&>(dev).try_invoke_async<AsyncCap>(AsyncCap::Request{ 5 });
    REQUIRE(fut.get().value == 10);
  }

  SECTION("try_invoke_async wraps synchronous capability into future when async impl missing") {
    auto fut = static_cast<IDevice&>(dev).try_invoke_async<SyncOnlyCap>(SyncOnlyCap::Request{ 9 });
    REQUIRE(fut.get().value == -9);
  }

  SECTION("const try_invoke_async works for capabilities with const-only implementation") {
    const TestDevice& cdev = dev;
    auto fut               = static_cast<const IDevice&>(cdev).try_invoke_async<ConstOnlyCap>(ConstOnlyCap::Request{ 8 });
    REQUIRE(fut.get().value == 50); // 8 + 42
  }

  SECTION("try_invoke_async throws CapabilityNotSupported for unsupported capability") {
    REQUIRE_THROWS_AS(static_cast<IDevice&>(dev).try_invoke_async<UnregisteredCap>(UnregisteredCap::Request{ 1 }), CapabilityNotSupported);

    try {
      (void)static_cast<IDevice&>(dev).try_invoke_async<UnregisteredCap>(UnregisteredCap::Request{ 1 });
      FAIL("Expected CapabilityNotSupported to be thrown");
    } catch (const CapabilityNotSupported& ex) {
      REQUIRE(ex.capability_type == std::type_index(typeid(UnregisteredCap)));
    }
  }
}

TEST_CASE("IDevice capabilities with multiple inputs/outputs", "[IDevice]") {
  TestDevice dev;

  SECTION("TwoInputOneOutputCap - two inputs, one output") {
    SECTION("non-const invoke multiplies inputs") {
      auto result = static_cast<IDevice&>(dev).invoke<TwoInputOneOutputCap>(TwoInputOneOutputCap::Request{ 5, 7 });
      REQUIRE(result.has_value());
      REQUIRE(result->result == 35); // 5 * 7
      REQUIRE(dev.two_input_one_output_calls == 1);
    }

    SECTION("const invoke adds offset to result") {
      const TestDevice& cdev = dev;
      auto initial_calls      = dev.two_input_one_output_calls;
      auto result             = static_cast<const IDevice&>(cdev).invoke<TwoInputOneOutputCap>(TwoInputOneOutputCap::Request{ 3, 4 });
      REQUIRE(result.has_value());
      REQUIRE(result->result == 22); // 3 * 4 + 10
      REQUIRE(dev.two_input_one_output_calls == initial_calls + 1);
    }

    SECTION("try_invoke works with two inputs") {
      auto resp = static_cast<IDevice&>(dev).try_invoke<TwoInputOneOutputCap>(TwoInputOneOutputCap::Request{ 6, 8 });
      REQUIRE(resp.result == 48); // 6 * 8
    }

    SECTION("invoke_async wraps two-input capability") {
      auto fut_opt = static_cast<IDevice&>(dev).invoke_async<TwoInputOneOutputCap>(TwoInputOneOutputCap::Request{ 2, 9 });
      REQUIRE(fut_opt.has_value());
      REQUIRE(fut_opt->get().result == 18); // 2 * 9
    }
  }

  SECTION("OneInputTwoOutputCap - one input, two outputs") {
    SECTION("non-const invoke returns doubled and squared") {
      auto result = static_cast<IDevice&>(dev).invoke<OneInputTwoOutputCap>(OneInputTwoOutputCap::Request{ 5 });
      REQUIRE(result.has_value());
      REQUIRE(result->doubled == 10);  // 5 * 2
      REQUIRE(result->squared == 25);  // 5 * 5
      REQUIRE(dev.one_input_two_output_calls == 1);
    }

    SECTION("const invoke adds offset to both outputs") {
      const TestDevice& cdev = dev;
      auto initial_calls      = dev.one_input_two_output_calls;
      auto result             = static_cast<const IDevice&>(cdev).invoke<OneInputTwoOutputCap>(OneInputTwoOutputCap::Request{ 4 });
      REQUIRE(result.has_value());
      REQUIRE(result->doubled == 9);   // 4 * 2 + 1
      REQUIRE(result->squared == 17);  // 4 * 4 + 1
      REQUIRE(dev.one_input_two_output_calls == initial_calls + 1);
    }

    SECTION("try_invoke works with two outputs") {
      auto resp = static_cast<IDevice&>(dev).try_invoke<OneInputTwoOutputCap>(OneInputTwoOutputCap::Request{ 7 });
      REQUIRE(resp.doubled == 14);  // 7 * 2
      REQUIRE(resp.squared == 49); // 7 * 7
    }

    SECTION("invoke_async wraps one-input-two-output capability") {
      auto fut_opt = static_cast<IDevice&>(dev).invoke_async<OneInputTwoOutputCap>(OneInputTwoOutputCap::Request{ 3 });
      REQUIRE(fut_opt.has_value());
      auto resp = fut_opt->get();
      REQUIRE(resp.doubled == 6);  // 3 * 2
      REQUIRE(resp.squared == 9);   // 3 * 3
    }
  }

  SECTION("TwoInputTwoOutputCap - two inputs, two outputs") {
    SECTION("non-const invoke returns sum and product") {
      auto result = static_cast<IDevice&>(dev).invoke<TwoInputTwoOutputCap>(TwoInputTwoOutputCap::Request{ 5, 3 });
      REQUIRE(result.has_value());
      REQUIRE(result->sum == 8);      // 5 + 3
      REQUIRE(result->product == 15);  // 5 * 3
      REQUIRE(dev.two_input_two_output_calls == 1);
    }

    SECTION("const invoke adds offset to both outputs") {
      const TestDevice& cdev = dev;
      auto initial_calls      = dev.two_input_two_output_calls;
      auto result             = static_cast<const IDevice&>(cdev).invoke<TwoInputTwoOutputCap>(TwoInputTwoOutputCap::Request{ 4, 6 });
      REQUIRE(result.has_value());
      REQUIRE(result->sum == 15);     // 4 + 6 + 5
      REQUIRE(result->product == 29); // 4 * 6 + 5
      REQUIRE(dev.two_input_two_output_calls == initial_calls + 1);
    }

    SECTION("try_invoke works with two inputs and two outputs") {
      auto resp = static_cast<IDevice&>(dev).try_invoke<TwoInputTwoOutputCap>(TwoInputTwoOutputCap::Request{ 8, 2 });
      REQUIRE(resp.sum == 10);     // 8 + 2
      REQUIRE(resp.product == 16);  // 8 * 2
    }

    SECTION("invoke_async wraps two-input-two-output capability") {
      auto fut_opt = static_cast<IDevice&>(dev).invoke_async<TwoInputTwoOutputCap>(TwoInputTwoOutputCap::Request{ 7, 9 });
      REQUIRE(fut_opt.has_value());
      auto resp = fut_opt->get();
      REQUIRE(resp.sum == 16);     // 7 + 9
      REQUIRE(resp.product == 63);  // 7 * 9
    }

    SECTION("zero inputs handled correctly") {
      auto result = static_cast<IDevice&>(dev).invoke<TwoInputTwoOutputCap>(TwoInputTwoOutputCap::Request{ 0, 0 });
      REQUIRE(result.has_value());
      REQUIRE(result->sum == 0);
      REQUIRE(result->product == 0);
    }

    SECTION("negative inputs handled correctly") {
      auto result = static_cast<IDevice&>(dev).invoke<TwoInputTwoOutputCap>(TwoInputTwoOutputCap::Request{ -3, 4 });
      REQUIRE(result.has_value());
      REQUIRE(result->sum == 1);      // -3 + 4
      REQUIRE(result->product == -12); // -3 * 4
    }
  }

  SECTION("ProcessStringsCap - complex types (strings and vectors)") {
    SECTION("non-const invoke processes strings with prefix") {
      ProcessStringsCap::Request req;
      req.words  = { "hello", "world", "test" };
      req.prefix = "pre_";

      auto result = static_cast<IDevice&>(dev).invoke<ProcessStringsCap>(req);
      REQUIRE(result.has_value());
      REQUIRE(result->processed_words.size() == 3);
      REQUIRE(result->processed_words[0] == "pre_hello");
      REQUIRE(result->processed_words[1] == "pre_world");
      REQUIRE(result->processed_words[2] == "pre_test");
      REQUIRE(result->concatenated_result == "pre_hello_world_test");
      REQUIRE(result->total_length == 14); // 5 + 5 + 4
      REQUIRE(dev.process_strings_calls == 1);
    }

    SECTION("const invoke adds CONST_ prefix and counts uppercase") {
      const TestDevice& cdev = dev;
      ProcessStringsCap::Request req;
      req.words  = { "Hello", "WORLD", "test" };
      req.prefix = "pre_";

      auto initial_calls = dev.process_strings_calls;
      auto result        = static_cast<const IDevice&>(cdev).invoke<ProcessStringsCap>(req);
      REQUIRE(result.has_value());
      REQUIRE(result->processed_words.size() == 3);
      REQUIRE(result->processed_words[0] == "CONST_pre_Hello");
      REQUIRE(result->processed_words[1] == "CONST_pre_WORLD");
      REQUIRE(result->processed_words[2] == "CONST_pre_test");
      REQUIRE(result->concatenated_result == "CONST_pre_Hello_WORLD_test");
      // total_length includes uppercase count: 5+5+4 (word lengths) + 6 (uppercase: H + WORLD) = 20
      REQUIRE(result->total_length == 20);
      REQUIRE(dev.process_strings_calls == initial_calls + 1);
    }

    SECTION("try_invoke works with complex types") {
      ProcessStringsCap::Request req;
      req.words  = { "foo", "bar" };
      req.prefix = "x_";

      auto resp = static_cast<IDevice&>(dev).try_invoke<ProcessStringsCap>(req);
      REQUIRE(resp.processed_words.size() == 2);
      REQUIRE(resp.processed_words[0] == "x_foo");
      REQUIRE(resp.processed_words[1] == "x_bar");
      REQUIRE(resp.concatenated_result == "x_foo_bar");
      REQUIRE(resp.total_length == 6); // 3 + 3
    }

    SECTION("invoke_async wraps complex type capability") {
      ProcessStringsCap::Request req;
      req.words  = { "async", "test" };
      req.prefix = "a_";

      auto fut_opt = static_cast<IDevice&>(dev).invoke_async<ProcessStringsCap>(req);
      REQUIRE(fut_opt.has_value());
      auto resp = fut_opt->get();
      REQUIRE(resp.processed_words.size() == 2);
      REQUIRE(resp.processed_words[0] == "a_async");
      REQUIRE(resp.processed_words[1] == "a_test");
      REQUIRE(resp.concatenated_result == "a_async_test");
      REQUIRE(resp.total_length == 9); // 5 + 4
    }

    SECTION("empty vector handled correctly") {
      ProcessStringsCap::Request req;
      req.words  = {};
      req.prefix = "empty_";

      auto result = static_cast<IDevice&>(dev).invoke<ProcessStringsCap>(req);
      REQUIRE(result.has_value());
      REQUIRE(result->processed_words.empty());
      REQUIRE(result->concatenated_result == "empty_");
      REQUIRE(result->total_length == 0);
    }

    SECTION("single element vector works") {
      ProcessStringsCap::Request req;
      req.words  = { "single" };
      req.prefix = "";

      auto result = static_cast<IDevice&>(dev).invoke<ProcessStringsCap>(req);
      REQUIRE(result.has_value());
      REQUIRE(result->processed_words.size() == 1);
      REQUIRE(result->processed_words[0] == "single");
      REQUIRE(result->concatenated_result == "single");
      REQUIRE(result->total_length == 6);
    }
  }
}
