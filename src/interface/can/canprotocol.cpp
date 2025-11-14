#include "canprotocol.hpp"

namespace eduart {

namespace com {

void CanProtocol::makeCanStdID(canid sysID, canid nodeID, canid& inputAddress, canid& outputAddress, canid& broadcastAddress) {
  canid sID  = sysID << 8;
  canid iBit = INPUTBIT << 7;
  canid oBit = OUTPUTBIT << 7;
  canid nID  = nodeID;

  inputAddress     = sID | iBit | nID;
  outputAddress    = sID | oBit | nID;
  broadcastAddress = sID | iBit | 0b0000000;
}

} // namespace com

} // namespace eduart