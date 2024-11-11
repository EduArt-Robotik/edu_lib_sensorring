#pragma once

#include <memory>
#include <array>
#include "SocketCANFD.hpp"
#include "heimann_htpa32.hpp"
#include "CustomTypes.hpp"

namespace sensor{

class BaseSensor : public com::SocketCANFDObserver{
    public:
        BaseSensor(std::shared_ptr<com::SocketCANFD> can_interface, canid_t canid, bool enable);
        ~BaseSensor();

        //void enableCallback();

        bool isEnabled() const;
        bool gotNewData() const;
        bool newDataAvailable() const;
        //void setEnable(bool enable);
        void clearDataFlag();
        
        void notify(const canfd_frame& frame) override;

        virtual void canCallback(const canfd_frame& frame) = 0;
        virtual void onClearDataFlag() = 0;

    protected:
        bool _new_data_available_flag;
        bool _new_data_in_buffer_flag;
        bool _new_measurement_ready_flag;
        SensorState _error;
        
    private:
        bool _enable_flag;

        canid_t  _canid_data;
        std::shared_ptr<com::SocketCANFD> _can_interface;
};

}; // namespace sensor