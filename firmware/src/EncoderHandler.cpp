#include "EncoderHandler.h"

/// @brief Constructor for the EncoderHandler class.
/// @param ENC_1 The pin number for the first encoder channel.
/// @param ENC_2 The pin number for the second encoder channel.
/// @param ENC_B The pin number for the button.
/// @param doubleClickSpeed The speed for detecting double clicks.
EncoderHandler::EncoderHandler(int ENC_1, int ENC_2, int ENC_B, int doubleClickSpeed)
    : _doubleClickSpeed(doubleClickSpeed)
{
  _encoder.begin(ENC_1, ENC_2);
  _button.attach(ENC_B, INPUT_PULLUP);
  _clicks.singleClick = false;
  _clicks.doubleClick = false;

  // Set some arbitrary default values
  _encoder.setLimits(0, 1, false);
  _min = 0;
  _max = 1;
}

/// @brief Poll the button state and update click information.
void EncoderHandler::pollButton()
{
  _button.update();
  // When the button is first pressed, register the click and start a timer to
  // determine if it is a double click.
  if (_button.pressed())
  {
    if (_clickTimer.hasPassed(_doubleClickSpeed))
    {
      _clicks.singleClick = true;
      _clickTimer.restart();
    }
    else
    {
      _clicks.doubleClick = true;
    }
  }
}

/// @brief Set the encoder value.
/// @param value The value to set.
void EncoderHandler::setEncoderValue(int value)
{
  _encoder.setValue(value);
}

/// @brief Get the current encoder value.
/// @return The current encoder value.
int EncoderHandler::getEncoderValue()
{
  return _encoder.getValue();
}

/// @brief Check if the encoder value has changed.
/// @return True if the encoder value has changed, false otherwise.
bool EncoderHandler::encoderValueChanged()
{
  return _encoder.valueChanged();
}

/// @brief Get the button press information.
/// @return The struct containing click information.
Clicks EncoderHandler::buttonPressed()
{
  Clicks click = _clicks;
  _clicks.singleClick = false;
  _clicks.doubleClick = false;
  return click;
}

/// @brief Set the encoder interval limits.
/// @param lowerLimit The lower limit to set.
/// @param upperLimit The upper limit to set.
/// @param periodic Whether the encoder should wrap around when reaching the limits.
void EncoderHandler::setEncoderInterval(int lowerLimit, int upperLimit, bool periodic)
{
  _encoder.setLimits(lowerLimit, upperLimit, periodic);
  _min = lowerLimit;
  _max = upperLimit;
}

/// @brief Set the double click speed.
/// @param doubleClickSpeed The double click speed in ms to set.
void EncoderHandler::setDoubleClickSpeed(int doubleClickSpeed)
{
  _doubleClickSpeed = doubleClickSpeed;
}

/// @brief Get the double click speed.
/// @return The double click speed.
int EncoderHandler::getDoubleClickSpeed()
{
  return _doubleClickSpeed;
}

/// @brief Get the minimum value of the encoder.
/// @return The minimum value of the encoder.
int EncoderHandler::getMin()
{
  return _min;
}

/// @brief Get the maximum value of the encoder.
/// @return The maximum value of the encoder.
int EncoderHandler::getMax()
{
  return _max;
}