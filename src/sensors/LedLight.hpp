#pragma once

#include "BaseSensor.hpp"
#include "Parameters.hpp"

namespace eduart {

namespace sensor {

class LedLight {
public:
  LedLight(LightParams params);
  ~LedLight();

  const LightParams& getParams() const;

private:
  const LightParams _params;

  int _canid_in;
  int _canid_out;
};

}

}