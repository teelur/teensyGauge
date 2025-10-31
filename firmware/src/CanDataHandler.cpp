#include "CanDataHandler.h"

#include <FlexCAN_T4.h>

FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> can;

constexpr uint32_t kBaseID = 1520; // This is set in megasquirt (typically 1520)

MegaCAN_broadcast_message_t CanDataHandler::_bCastMsg = {};
MegaCAN CanDataHandler::_mega_can(kBaseID);

/// @brief Initializes the CAN bus.
/// @param canBaud The baud rate for the CAN bus.
void CanDataHandler::initCan(int canBaud)
{
  can.begin();
  can.setBaudRate(canBaud);
  can.setMaxMB(FLEXCAN_MAILBOX::MB16); // sets maximum number of mailboxes for FlexCAN_T4 usage
  can.enableFIFO();
  can.enableFIFOInterrupt();
  can.onReceive(CanDataHandler::canMShandler); // when a CAN message is received, runs the canMShandler function
  can.mailboxStatus();
}

/// @brief Polls the CAN bus for events.
void CanDataHandler::pollCan()
{
  can.events();
}

/// @brief Retrieves gauge data based on the provided gauges.
/// @param gauges A vector of GaugeData enums specifying which gauges to retrieve.
std::vector<std::pair<GaugeData, String>> CanDataHandler::getGaugeData(const std::vector<GaugeData>& gauges)
{
  std::vector<std::pair<GaugeData, String>> data;

  for (auto gauge : gauges)
  {
    switch (gauge)
    {
    case GaugeData::kRPM:
      data.push_back({GaugeData::kRPM, _bCastMsg.rpm});
      break;
    case GaugeData::kAFR:
      data.push_back({GaugeData::kAFR, _bCastMsg.afr1_old});
      break;
    case GaugeData::kMAP:
      data.push_back({GaugeData::kMAP, _bCastMsg.map});
      break;
    case GaugeData::kMAT:
      data.push_back({GaugeData::kMAT, _bCastMsg.mat});
      break;
    case GaugeData::kVoltage:
      data.push_back({GaugeData::kVoltage, _bCastMsg.batt});
      break;
    case GaugeData::kTPS:
      data.push_back({GaugeData::kTPS, _bCastMsg.tps});
      break;
    case GaugeData::kCLT:
      data.push_back({GaugeData::kCLT, _bCastMsg.clt});
      break;
    case GaugeData::kFan:
      data.push_back({GaugeData::kFan, bitRead(_bCastMsg.status6, int(Status6::kFan))});
      break;
    case GaugeData::kWUE:
      data.push_back({GaugeData::kWUE, bitRead(_bCastMsg.engine, int(Engine::kWUE))});
    default:
      break;
    };
  }

  return data;
}

/// @brief Routine used by FlexCAN for retrieving the received CAN data.
/// @param msg The received CAN message.
void CanDataHandler::canMShandler(const CAN_message_t& msg)
{
  if (!msg.flags.extended)
  {
    // broadcast data from MS does not use extended flag, therefore this should be broadcast data from MS
    _mega_can.getBCastData(msg.id, msg.buf, _bCastMsg);
  }
}
