#pragma once

#include "MegaSquirtInfo.h"

#include <DisplayHandler.h>
#include <EncoderHandler.h>

#include <unordered_map>

class CanDataHandler;
class EncoderHandler;

// This is used to track the current state of the UI.
// - Idle consists of a scrollable list of views (dashboard, quad, dual, etc.).
// - Selecting a given view will allow you to scroll through the items on that view.
// - Selecting an item will allow you to scroll through a list of items to replace in the current view.
enum State : int
{
  kIdle = 0,
  kViewSelected = 1,
  kItemSelected = 2,
  kSettingsSelected = 3,
};

struct StateInfo
{
  int index = 0;
};

class StateManager
{
public:
  StateManager() = delete;
  StateManager(EncoderHandler& encoderHandler, CanDataHandler& canDataHandler, DisplayHandler& displayHandler);

  void poll();
  void serveData();

private:
  EncoderHandler& _encoderHandler;
  CanDataHandler& _canDataHandler;
  DisplayHandler& _displayHandler;

  State _menuState;

  GaugeView _currentView;
  int _currentGaugeCursorIndex;

  std::vector<GaugeData> _currentDashboardGauges;
  std::vector<GaugeData> _currentQuadGauges;
  std::vector<GaugeData> _currentDualGauges;
  GaugeData _currentSingleGauge;

  std::unordered_map<State, StateInfo> _stateMap;

  void _handleScroll(int newValue);
  void _handleItemSelectedScroll(int newValue);
  void _handleClick(Clicks clicks);
  void _handleIdleClick();
  void _handleViewSelectedClick(Clicks clicks);
  void _handleItemSelectedClick(Clicks clicks);
  void _updateEncoder(int initialValue);
  std::vector<std::pair<GaugeData, String>> _loadStateData(GaugeView state);
  std::unordered_map<State, StateInfo>::iterator _getCurrentStateInfo(State currentState);
};