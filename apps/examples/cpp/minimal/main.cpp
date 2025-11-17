/*
 * Minimal cpp example of using the sensorring library
 */

#include <thread>
#include <chrono>
#include <iomanip>
#include <iostream>

#include <sensorring/logger/Logger.hpp>
#include <sensorring/MeasurementManager.hpp>

#include "MeasurementProxy.hpp" 

using namespace std::chrono_literals;

int main(char argc, char* argv[])
{
    std::cout << "==========================" << std::endl;
    std::cout << "Minimal sensorring example" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << std::endl;

    eduart::manager::ManagerParams params;
    {
        eduart::sensor::TofSensorParams tof;
        tof.user_idx = 0;
        tof.enable = true;

        eduart::sensor::SensorBoardParams board;
        board.tof_params = tof;
        
        eduart::bus::BusParams bus;
        bus.interface_name = "0x1731a1f1";
        bus.type = eduart::com::InterfaceType::USBTINGO;
        bus.board_param_vec.push_back(board);

        eduart::ring::RingParams ring;
        ring.bus_param_vec.push_back(bus);

        params.ring_params = ring;
    }

    auto proxy = std::make_unique<eduart::MeasurementProxy>();
    eduart::logger::Logger::getInstance()->registerClient(proxy.get());

    auto manager = std::make_unique<eduart::manager::MeasurementManager>(params);
    manager->registerClient(proxy.get());
    manager->startMeasuring();

    while(!proxy->gotFirstMeasurement()) {

    };

    std::cout << std::endl << "Printing measurement rate:" << std::endl;
    while(true) {
        std::cout   << "Current rate: "
                    << std::fixed << std::setprecision(2)
                    << std::setw(5) << proxy->getRate()
                    << " Hz\r"
                    << std::flush;
        std::this_thread::sleep_for(1s);
    }

    manager->stopMeasuring();
    return 0;
}