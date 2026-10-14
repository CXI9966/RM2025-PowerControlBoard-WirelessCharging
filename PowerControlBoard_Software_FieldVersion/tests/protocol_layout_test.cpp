#include "Communication.hpp"
#include <stddef.h>

static_assert(sizeof(RxData) == 8, "command layout changed");
static_assert(sizeof(TxData) == 8, "float feedback layout changed");
static_assert(sizeof(TxDataNew) == 8, "compact feedback layout changed");
static_assert(offsetof(RxData, refereePowerLimit) == 1, "command budget moved");
static_assert(offsetof(RxData, refereeEnergyBuffer) == 3, "command energy moved");
static_assert(offsetof(RxData, activeChargingLimitRatio) == 5, "charging ratio moved");
static_assert(offsetof(RxData, resv2) == 6, "reserved bytes moved");
static_assert(offsetof(TxData, chassisPower) == 1, "float load power moved");
static_assert(offsetof(TxData, chassisPowerLimit) == 5, "float power limit moved");
static_assert(offsetof(TxData, capEnergy) == 7, "float energy moved");
static_assert(offsetof(TxDataNew, chassisPower) == 1, "compact load power moved");
static_assert(offsetof(TxDataNew, refereePower) == 3, "battery power moved");
static_assert(offsetof(TxDataNew, chassisPowerLimit) == 5, "compact power limit moved");
static_assert(offsetof(TxDataNew, capEnergy) == 7, "compact energy moved");
static_assert(CAN_CMD_ID <= 0x7FFU && CAN_FEEDBACK_ID <= 0x7FFU &&
              CAN_FEEDBACK_NEW_ID <= 0x7FFU, "CAN IDs are not standard IDs");

#ifdef WPT_HARDWARE
#error "JointSuperCap verification must not enable wireless charging"
#endif

// Extracted with objcopy and checked against independent little-endian bytes.
extern "C" const RxData commandLayoutProbe
    __attribute__((used, section(".protocol_probe"))) = {1, 0, 0, 0, 0, 1, 100, 57, 255, 0};
