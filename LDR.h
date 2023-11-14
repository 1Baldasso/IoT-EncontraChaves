#ifndef _LDR_H_
#define _LDR_H_

#include <SinricProDevice.h>
#include <Capabilities/RangeController.h>

class LDR 
: public SinricProDevice
, public RangeController<LDR> {
  friend class RangeController<LDR>;
public:
  LDR(const String &deviceId) : SinricProDevice(deviceId, "LDR") {};
};

#endif
