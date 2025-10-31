#pragma once

#include <Arduino.h>
#include <vector>

enum class GaugeData : int
{
  kRPM = 0,
  kAFR = 1,
  kCLT = 2,
  kMAP = 3,
  kMAT = 4,
  kBoost = 5,
  kVoltage = 6,
  kTPS = 7,
  kKnock = 8,
  kBarometer = 9,
  kEGOCorrection = 10,
  kIAC = 11,
  kSparkDwell = 12,
  kBoostDuty = 13,
  kIdleTarget = 14,
  kAfrTarget = 15,
  kTiming = 16,
  kFan = 17,
  kWUE = 18
};

enum class Engine : uint8_t
{
  kReady = 0,
  kCrank = 1,
  kASE = 2,
  kWUE = 3,
  kTpsAccelEnrich = 4,
  kTpsDecelEnrich = 5,
  kMapAccelEnrich = 6,
  kMapDecelEnrich = 7,
};

enum class Status1 : uint8_t
{
  kNeedBurn = 0,
  kLostData = 1,
  kConfigError = 2,
  kRPMSynced = 3,
  kSynclatch = 4,
  kFuelTableSw = 5,
  kSparkTableSw = 6,
  kFullRPMSynced = 7,
};

enum class Status2 : uint8_t
{
  kN2OStage1 = 0,
  kN2OStage2 = 1,
  kLaunchIn = 2,
  kLaunch = 3,
  kFlatShift = 4,
  kSparkCut = 5,
  kOverBoost = 6,
  kCLIdle = 7,
};

enum class Status3 : uint8_t
{
  kFuelCut = 0,
  kTLog = 1,
  k3Step = 2,
  kTestMode = 3,
  k3StepIn = 4,
  kSoftLimit = 5,
  kSeqShift = 6,
  kLaunchOn = 7,
};

enum class Status6 : uint8_t
{
  kEGTWarn = 0,
  kEGTShutdown = 1,
  kAFRWarn = 2,
  kAFRShutdown = 3,
  kIdleVE = 4,
  kIdleAdvance = 5,
  kFan = 6,
  kMAPSampleError = 7,
};

enum class Status7 : uint8_t
{
  kVVT1Error = 0,
  kVVT2Error = 1,
  kVVT3Error = 2,
  kVVT4Error = 3,
  kKnock = 4,
  kAC = 5,
  kCEL = 6,
  kLimpMode = 7,
};

enum class Status8 : uint8_t
{
  kInjectorsDisableMode = 0,
  kSparkDisableMode = 1,
  kWaterInjectionLow = 2,
  kPW4xMode = 3,
  kVSSLaunchLockout = 4,
  kVSSLaunch = 5,
};

const std::vector<String> GaugeLabels = {
    "RPM",      "AFR", "Coolant",    "MAP",        "MAT",        "Boost",      "Volts",  "TPS", "Knock", "Baro",
    "EGO Corr", "IAC", "Sprk Dwell", "Boost Duty", "Idl Target", "AFR Target", "Timing", "Fan", "WUE"};
