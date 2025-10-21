#include "canprotocol.hpp"

namespace eduart {

namespace com {

void CanProtocol::makeCanStdID (canid_t sysID, canid_t nodeID, canid_t &inputAddress, canid_t &outputAddress, canid_t &broadcastAddress) {
  canid_t sID  = sysID << 8;
  canid_t iBit = INPUTBIT << 7;
  canid_t oBit = OUTPUTBIT << 7;
  canid_t nID  = nodeID;

  inputAddress     = sID | iBit | nID;
  outputAddress    = sID | oBit | nID;
  broadcastAddress = sID | iBit | 0b0000000;
}

}

}