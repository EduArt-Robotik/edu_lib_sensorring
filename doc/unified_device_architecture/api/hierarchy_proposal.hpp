// Minimal example: ownership hierarchy + flat APIs
#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <type_traits>

class Device {
public:
    virtual ~Device() = default;
    virtual const char* kind() const = 0;
    // device-specific interface...
};

class SensorBoard {
public:
    // Board owns devices
    std::vector<std::unique_ptr<Device>> devices;

    void addDevice(std::unique_ptr<Device> d) {
        devices.push_back(std::move(d));
    }

    const std::vector<std::unique_ptr<Device>>& getDevices() const noexcept {
        return devices;
    }
};

class Bus {
public:
    // Bus owns boards
    std::vector<std::unique_ptr<SensorBoard>> boards;

    void addBoard(std::unique_ptr<SensorBoard> b) {
        boards.push_back(std::move(b));
    }

    const std::vector<std::unique_ptr<SensorBoard>>& getBoards() const noexcept {
        return boards;
    }
};

class Ring {
public:
    // Ring owns busses
    std::vector<std::unique_ptr<Bus>> buses;

    void addBus(std::unique_ptr<Bus> b) {
        buses.push_back(std::move(b));
    }

    const std::vector<std::unique_ptr<Bus>>& getBuses() const noexcept {
        return buses;
    }

    // Visitor-style: call fn(Device&) for each device
    template<typename Fn>
    void for_each_device(Fn&& fn) const {
        for (const auto& bus_ptr : buses) {
            for (const auto& board_ptr : bus_ptr->getBoards()) {
                for (const auto& dev_ptr : board_ptr->getDevices()) {
                    std::invoke(fn, *dev_ptr);
                }
            }
        }
    }

    // Snapshot-style: returns a flat vector of non-owning pointers.
    // Cheap to produce for small-to-medium device counts. Caller must not use pointers
    // after Ring mutation/destruction unless you use shared_ptr.
    std::vector<Device*> devices_snapshot() const {
        std::vector<Device*> out;
        // optional: reserve if you can estimate device count
        for (const auto& bus_ptr : buses) {
            for (const auto& board_ptr : bus_ptr->getBoards()) {
                for (const auto& dev_ptr : board_ptr->getDevices()) {
                    out.push_back(dev_ptr.get());
                }
            }
        }
        return out;
    }

    // Typed filtered visitor: only call fn(T&) for devices where dynamic_cast<T*> succeeds.
    template<typename T, typename Fn>
    void for_each_device_of_type(Fn&& fn) const {
        static_assert(std::is_base_of<Device, T>::value, "T must derive from Device");
        for_each_device([&](Device& d) {
            if (auto t = dynamic_cast<T*>(&d)) std::invoke(fn, *t);
        });
    }
};