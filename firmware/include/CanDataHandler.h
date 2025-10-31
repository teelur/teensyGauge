#pragma once

#include "MegaSquirtInfo.h"

#include <FlexCAN_T4.h>
#include <MegaCAN.h>

#include <utility>
#include <vector>

extern FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can;

class CanDataHandler
{
public:
  CanDataHandler() = default;

  void pollCan();

  std::vector<std::pair<GaugeData, String>> getGaugeData(const std::vector<GaugeData>& data);

  static void initCan(int canBaud);
  static void canMShandler(const CAN_message_t& msg);

private:
  static MegaCAN _mega_can;
  static MegaCAN_broadcast_message_t _bCastMsg;
};