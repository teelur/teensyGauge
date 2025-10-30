#include "StateManager.h"

#include <CanDataHandler.h>

#include <utility>
#include <vector>

/// @brief Constructor for the StateManager class.
/// @param encoderHandler The encoder handler instance.
/// @param canDataHandler The CAN data handler instance.
/// @param displayHandler The display handler instance.
StateManager::StateManager(EncoderHandler& encoderHandler, CanDataHandler& canDataHandler,
                           DisplayHandler& displayHandler)
    : _encoderHandler(encoderHandler), _canDataHandler(canDataHandler), _displayHandler(displayHandler)
{
  _currentView = _displayHandler.getCurrentView();

  // Encoder needs to be initialized in the Idle view
  _encoderHandler.setEncoderInterval(int(GaugeView::kGaugeMin), int(GaugeView::kGaugeMax), true);
  _encoderHandler.setEncoderValue(int(_currentView));

  // Need to feed the display some initial data
  _currentDashboardGauges = _displayHandler.getCurrentDashboardGauges();
  _currentQuadGauges = _displayHandler.getCurrentQuadGauges();
  _currentDualGauges = _displayHandler.getCurrentDualGauges();
  _currentSingleGauge = _displayHandler.getCurrentSingleGauge();

  _displayHandler.setCurrentData(_loadStateData(_currentView));

  StateInfo info;
  _stateMap.insert(std::make_pair(kIdle, info));
}

/// @brief Polls the encoder and CAN data, and handles user input.
void StateManager::poll()
{
  _canDataHandler.pollCan();
  _encoderHandler.pollButton();

  Clicks buttonPressed = _encoderHandler.buttonPressed();
  _currentView = _displayHandler.getCurrentView();
  _currentIndex = _encoderHandler.getEncoderValue();

  // Handle user input
  if (_encoderHandler.encoderValueChanged())
  {
    _scrollGauge(_currentIndex);
  }

  if (buttonPressed.singleClick || buttonPressed.doubleClick)
  {
    _handleClick(buttonPressed);
  }
}

/// @brief Retrieves the CAN data for the currently selected gauges and serves to the display.
void StateManager::serveData()
{
  auto currentData = _displayHandler.getCurrentData();
  std::vector<GaugeData> currentGauges;

  for (auto gauge : currentData)
  {
    currentGauges.push_back(gauge.first);
  }

  currentData = _canDataHandler.getGaugeData(currentGauges);
  _displayHandler.setCurrentData(currentData);

  _displayHandler.display();
}

/// @brief Updates the gauge state to a new view, and feeds all necessary info to init that view.
/// @param newState The new gauge view state.
void StateManager::_scrollGauge(int newState)
{
  switch (_menuState)
  {
  case kIdle:
    // Scroll through the list of views
    _displayHandler.setCurrentView(static_cast<GaugeView>(newState));
    _displayHandler.setCurrentData(_loadStateData(static_cast<GaugeView>(newState)));
    break;
  case kViewSelected:
    // Scroll through the individual gauges on a view
    _displayHandler.moveGaugeCursor(newState);
    break;
  case kItemSelected:
    break;
  default:
    Serial.println("This state does not support scrolling!");
  }
}

/// @brief Handles user clicks based on the current menu state.
/// @param clicks The click events that triggered the action.
void StateManager::_handleClick(Clicks clicks)
{
  switch (_menuState)
  {
  case kIdle:
    _handleIdleClick();
    break;
  case kViewSelected:
    _handleViewSelectedClick(clicks);
    break;
  case kItemSelected:
    _handleItemSelectedClick(clicks);
    break;
  default:
    Serial.println("This state does not support selection!");
  }
}

/// @brief Handles click events in the Idle state.
void StateManager::_handleIdleClick()
{
  switch (_currentView)
  {
  case GaugeView::kQuadGauge:
  case GaugeView::kDualGauge: {
    auto currentStateInfo = _getCurrentStateInfo(_menuState);
    currentStateInfo->second.index = _currentIndex;

    _menuState = kViewSelected;
    _updateEncoder(0);

    _displayHandler.moveGaugeCursor(0);
    _displayHandler.createBackArrow();
    break;
  }
  default:
    Serial.println("Select not supported on this gauge view!");
  }
}

/// @brief Handles click events in the View Selected state.
/// @param clicks The click events that triggered the action.
void StateManager::_handleViewSelectedClick(Clicks clicks)
{
  if (clicks.singleClick)
  {
    // Back arrow goes back up to top level view
    if (_encoderHandler.getMax() == _currentIndex)
    {
      _menuState = kIdle;

      auto currentStateInfo = _getCurrentStateInfo(_menuState);
      _updateEncoder(currentStateInfo->second.index);

      _displayHandler.clearGaugeCursor();
      _displayHandler.clearBackArrow();
    }
    else
    {
      // Save the index of the previous screen
      auto currentStateInfo = _getCurrentStateInfo(_menuState);
      currentStateInfo->second.index = _currentIndex;

      _menuState = kItemSelected;
      _updateEncoder(0);

      _displayHandler.clearBackArrow();
    }
  }
}

/// @brief Handles click events in the Item Selected state.
/// @param clicks The click events that triggered the action.
void StateManager::_handleItemSelectedClick(Clicks clicks)
{
  if (clicks.singleClick)
  {
    _menuState = kViewSelected;

    auto currentStateInfo = _getCurrentStateInfo(_menuState);
    _updateEncoder(currentStateInfo->second.index);

    _displayHandler.createBackArrow();
  }
}

/// @brief Updates the encoder settings based on the current menu state.
/// @param initialValue The initial value to set the encoder to.
void StateManager::_updateEncoder(int initialValue)
{
  // Interval is dependent on the current state
  if (kIdle == _menuState)
  {
    _encoderHandler.setEncoderInterval(int(GaugeView::kGaugeMin), int(GaugeView::kGaugeMax), true);
  }
  else if (kViewSelected == _menuState)
  {
    switch (_currentView)
    {
    case GaugeView::kQuadGauge:
      _encoderHandler.setEncoderInterval(0, 4, true);
      break;
    case GaugeView::kDualGauge:
      _encoderHandler.setEncoderInterval(0, 2, true);
      break;
    default:
      Serial.println("Encoder not valid for this view!");
    }
  }
  else if (kItemSelected == _menuState)
  {
    _encoderHandler.setEncoderInterval(0, 1, true);
  }

  _encoderHandler.setEncoderValue(initialValue);
}

/// @brief Loads the gauge data for a given state/view.
/// @param state The gauge view state to load data for.
std::vector<std::pair<GaugeData, String>> StateManager::_loadStateData(GaugeView state)
{
  std::vector<GaugeData> currentGauges;

  switch (state)
  {
  case GaugeView::kDashboard:
    currentGauges = _currentDashboardGauges;
    break;
  case GaugeView::kQuadGauge:
    currentGauges = _currentQuadGauges;
    break;
  case GaugeView::kDualGauge:
    currentGauges = _currentDualGauges;
    break;
  case GaugeView::kSingleGauge:
    currentGauges = {_currentSingleGauge};
    break;
  default:
    Serial.println("No stored data for this given state!");
  }

  return _canDataHandler.getGaugeData(currentGauges);
}

/// @brief Retrieves the StateInfo iterator for the given state, creating a new entry if it doesn't exist.
/// @param currentState The state to retrieve information for.
/// @return An iterator to the StateInfo for the given state.
std::unordered_map<State, StateInfo>::iterator StateManager::_getCurrentStateInfo(State currentState)
{
  auto info = _stateMap.find(currentState);

  if (_stateMap.end() == info)
  {
    StateInfo stateInfo;
    _stateMap.insert(std::make_pair(currentState, stateInfo));
    info = _stateMap.find(currentState);
  }

  return info;
}
