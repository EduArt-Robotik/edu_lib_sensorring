#pragma once

#include <memory>
#include <array>
#include "ComInterface.hpp"
#include "heimann_htpa32.hpp"
#include "CustomTypes.hpp"

namespace sensor{

class BaseSensor : public com::ComObserver{
    public:
        BaseSensor(std::shared_ptr<com::ComInterface> interface, com::Endpoint target, bool enable);
        ~BaseSensor();

        //void enableCallback();

        bool isEnabled() const;
        bool gotNewData() const;
        bool newDataAvailable() const;
        //void setEnable(bool enable);
        void clearDataFlag();
        
        void notify(const com::Endpoint source, const std::vector<uint8_t>& data) override;

        virtual void canCallback(const com::Endpoint source, const std::vector<uint8_t>& data) = 0;
        virtual void onClearDataFlag() = 0;

    protected:
        bool _new_data_available_flag;
        bool _new_data_in_buffer_flag;
        bool _new_measurement_ready_flag;
        SensorState _error;
        
    private:
        bool _enable_flag;

        com::Endpoint _target;
        std::shared_ptr<com::ComInterface> _interface;
};

}; // namespace sensor