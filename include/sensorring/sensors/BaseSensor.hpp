#pragma once

#include <memory>
#include <array>
#include "ComInterface.hpp"
#include "ComEndpoints.hpp"
#include "heimann_htpa32.hpp"
#include "CustomTypes.hpp"

namespace eduart{

namespace sensor{

class BaseSensor : public com::ComObserver{
    public:
        BaseSensor(com::ComInterface* interface, com::ComEndpoint target, std::size_t idx, bool enable);
        ~BaseSensor();

        //void enableCallback();
        
        std::size_t getIdx() const;
        bool isEnabled() const;
        bool gotNewData() const;
        bool newDataAvailable() const;
        //void setEnable(bool enable);
        void clearDataFlag();
        
        void notify(const com::ComEndpoint source, const std::vector<uint8_t>& data) override;

        virtual void canCallback(const com::ComEndpoint source, const std::vector<uint8_t>& data) = 0;
        virtual void onClearDataFlag() = 0;

    protected:
        std::size_t _idx;
        SensorState _error;

        bool _new_data_available_flag;
        bool _new_data_in_buffer_flag;
        bool _new_measurement_ready_flag;
        
    private:
        bool _enable_flag;

        com::ComEndpoint _target;
        com::ComInterface* _interface;
};

}

}