#include "DisplayHandler.h"

/// @brief Constructor for the DisplayHandler class.
/// @param _tft_RST The reset pin for the TFT display.
/// @param _tft_DC The data/command pin for the TFT display.
/// @param _tft_CS The chip select pin for the TFT display.
/// @param _screenHeight The height of the display screen.
/// @param _screenWidth The width of the display screen.
DisplayHandler::DisplayHandler(int _tft_RST, int _tft_DC, int _tft_CS, int _screenHeight, int _screenWidth)
    : _screenHeight(_screenHeight), _screenWidth(_screenWidth), _tft(_tft_CS, _tft_DC, _tft_RST) // Hardware SPI
{
  // Initialize gauge cursor index to -1 (no selection).
  _gaugeCursorIndex = -1;

  // GaugeMin and GaugeMax window the selectable gauges. Stuff in development can be put outside of this window
  // temporarily.
  _currentGaugeView = GaugeView::kGaugeMin;

  // We need to draw the first gauge.
  _gaugeViewUpdated = true;

  // Default gauges for each view
  // TODO: Load saved user preferences here
  _currentDashboardGauges = {GaugeData::kAFR,     GaugeData::kCLT, GaugeData::kMAT, GaugeData::kMAP,
                             GaugeData::kVoltage, GaugeData::kFan, GaugeData::kWUE};
  _currentQuadGauges = {GaugeData::kRPM, GaugeData::kTPS, GaugeData::kMAP, GaugeData::kCLT};
  _currentDualGauges = {GaugeData::kRPM, GaugeData::kTPS};
  _currentSingleGauge = GaugeData::kRPM;

  // TODO: Figure out how I want to use this for caching gauge selections
  GaugeInfo info;
  _gaugeMap.insert(std::make_pair(_currentGaugeView, info));
}

/// @brief Displays the startup screen on the TFT display.
void DisplayHandler::displayStartupScreen()
{
  _tft.begin();
  clearScreen();
  _tft.drawBitmap(20, 98, miata_logo, 200, 44, GC9A01A_RED);
}

/// @brief Displays the current gauge view on the TFT display.
void DisplayHandler::display()
{
  // If we select a new gauge, we need to redraw EVERYTHING.
  // Otherwise we are just refreshing the data.
  switch (_currentGaugeView)
  {
  case GaugeView::kDashboard:
    _gaugeViewUpdated ? _drawDashboard() : _refreshDashboard();
    break;
  case GaugeView::kQuadGauge:
    _gaugeViewUpdated ? _drawQuad() : _refreshQuad();
    break;
  case GaugeView::kDualGauge:
    _gaugeViewUpdated ? _drawDual() : _refreshDual();
    break;
  case GaugeView::kSingleGauge:
    _gaugeViewUpdated ? _drawSingle() : _refreshSingle();
    break;
  default:
    Serial.print(int(_currentGaugeView));
    Serial.print(" is not a valid gauge index!\n");
  }

  _gaugeViewUpdated = false;
}

/// @brief Clears the TFT display screen.
void DisplayHandler::clearScreen()
{
  _tft.fillScreen(GC9A01A_BLACK);
}

/// @brief Draws a back arrow used for unselecting gauge views.
void DisplayHandler::createBackArrow()
{
  switch (_currentGaugeView)
  {
  case GaugeView::kQuadGauge:
  case GaugeView::kDualGauge:
  case GaugeView::kSingleGauge: {
    _drawBackArrow(GC9A01A_WHITE, GC9A01A_BLACK);
    break;
  }
  default:
    Serial.println("Back arrow not supported on this view!");
  }
}

/// @brief Clears the back arrow from the display.
void DisplayHandler::clearBackArrow()
{
  switch (_currentGaugeView)
  {
  case GaugeView::kQuadGauge:
  case GaugeView::kDualGauge:
  case GaugeView::kSingleGauge: {
    _drawBackArrow(GC9A01A_BLACK, GC9A01A_BLACK);
    break;
  }
  default:
    Serial.println("Back arrow not supported on this view!");
  }
}

/// @brief Retrieves the current gauge cursor index.
/// @return The current gauge cursor index.
int DisplayHandler::getCurrentGaugeCursorIndex()
{
  return _gaugeCursorIndex;
}

/// @brief Moves the cursor from the current gauge to the provided new index.
/// @param gaugeIndex The index of the gauge to move the cursor to.
void DisplayHandler::moveGaugeCursor(int gaugeIndex)
{
  switch (_currentGaugeView)
  {
  case GaugeView::kQuadGauge:
    _highlightQuadGauge(GC9A01A_WHITE, GC9A01A_BLACK);
    _gaugeCursorIndex = gaugeIndex;
    _highlightQuadGauge(GC9A01A_BLACK, GC9A01A_WHITE);
    break;
  case GaugeView::kDualGauge:
    _highlightDualGauge(GC9A01A_WHITE, GC9A01A_BLACK);
    _gaugeCursorIndex = gaugeIndex;
    _highlightDualGauge(GC9A01A_BLACK, GC9A01A_WHITE);
    break;
  default:
    Serial.println("Cursor not supported on this view!");
  }
}

/// @brief Clears the gauge cursor from the display.
void DisplayHandler::clearGaugeCursor()
{
  switch (_currentGaugeView)
  {
  case GaugeView::kQuadGauge:
    _highlightQuadGauge(GC9A01A_WHITE, GC9A01A_BLACK);
    break;
  case GaugeView::kDualGauge:
    _highlightDualGauge(GC9A01A_WHITE, GC9A01A_BLACK);
    break;
  default:
    Serial.println("Cursor not supported on this view!");
  }
  _gaugeCursorIndex = -1;
}

/// @brief Updates the gauge data and caches the old data for display refreshing.
/// @param newData The new gauge data to set.
void DisplayHandler::setCurrentData(std::vector<std::pair<GaugeData, String>> newData)
{
  _oldData = _currentData;
  _currentData = newData;
  _dataUpdated = true;
}

/// @brief Retrieves the current gauge data.
/// @return The current gauge data.
std::vector<std::pair<GaugeData, String>> DisplayHandler::getCurrentData()
{
  return _currentData;
}

/// @brief Sets the current gauge view.
/// @param newGauge The new gauge view to set.
void DisplayHandler::setCurrentView(GaugeView newGauge)
{
  _currentGaugeView = newGauge;
  _gaugeViewUpdated = true;
}

/// @brief Retrieves the current gauge view.
/// @return The current gauge view.
GaugeView DisplayHandler::getCurrentView()
{
  return _currentGaugeView;
}

/// @brief Sets the current dashboard gauges.
/// @param gauges The new dashboard gauges to set.
void DisplayHandler::setCurrentDashboardGauges(std::vector<GaugeData> gauges)
{
  _currentDashboardGauges = gauges;
}

/// @brief Retrieves the current dashboard gauges.
/// @return The current dashboard gauges.
std::vector<GaugeData> DisplayHandler::getCurrentDashboardGauges()
{
  return _currentDashboardGauges;
}

/// @brief Sets the current quad gauges.
/// @param gauges The new quad gauges to set.
void DisplayHandler::setCurrentQuadGauges(std::vector<GaugeData> gauges)
{
  _currentQuadGauges = gauges;
}

/// @brief Retrieves the current quad gauges.
/// @return The current quad gauges.
std::vector<GaugeData> DisplayHandler::getCurrentQuadGauges()
{
  return _currentQuadGauges;
}

/// @brief Sets the current dual gauges.
/// @param gauges The new dual gauges to set.
void DisplayHandler::setCurrentDualGauges(std::vector<GaugeData> gauges)
{
  _currentDualGauges = gauges;
}

/// @brief Retrieves the current dual gauges.
/// @return The current dual gauges.
std::vector<GaugeData> DisplayHandler::getCurrentDualGauges()
{
  return _currentDualGauges;
}

/// @brief Sets the current single gauge.
/// @param gauge The new single gauge to set.
void DisplayHandler::setCurrentSingleGauge(GaugeData gauge)
{
  _currentSingleGauge = gauge;
}

/// @brief Retrieves the current single gauge.
/// @return The current single gauge.
GaugeData DisplayHandler::getCurrentSingleGauge()
{
  return _currentSingleGauge;
}

/// @brief Refreshes the data on a by digit basis given the data center X coordinate and top Y coordinate.
/// @param dataIndex The index of the data to refresh.
/// @param fontSize The font size to use for the text.
/// @param cursorX The X coordinate of the text cursor.
/// @param cursorY The Y coordinate of the text cursor.
void DisplayHandler::_refreshData(int dataIndex, FontSize fontSize, int cursorX, int cursorY)
{
  _tft.setTextSize(int(fontSize));

  if ((unsigned int)dataIndex >= _currentData.size())
  {
    Serial.println("Data index outside of range!");
    return;
  }

  // To avoid flickering:
  // - Only update the data if it has changed
  // - Black out only the old data pixels

  // We need to redraw everything if the data length changes.
  if (_oldData[dataIndex].second.length() != _currentData[dataIndex].second.length())
  {
    _tft.setTextColor(GC9A01A_BLACK);
    _tft.setCursor(cursorX - _getCenterOffset(fontSize, _oldData[dataIndex].second.length()), cursorY);
    _tft.println(_oldData[dataIndex].second);

    _tft.setTextColor(GC9A01A_WHITE);
    _tft.setCursor(cursorX - _getCenterOffset(fontSize, _currentData[dataIndex].second.length()), cursorY);
    _tft.println(_currentData[dataIndex].second);
  }
  // Otherwise only draw changed digits
  else
  {
    int dataStart = _getCenterOffset(fontSize, _oldData[dataIndex].second.length());
    for (uint8_t i = 0; i < _oldData[dataIndex].second.length(); i++)
    {
      if (_oldData[dataIndex].second.charAt(i) != _currentData[dataIndex].second.charAt(i))
      {
        _tft.setTextColor(GC9A01A_BLACK);
        _tft.setCursor(cursorX - dataStart + (_getFontWidth(fontSize) * i), cursorY);
        _tft.println(_oldData[dataIndex].second.charAt(i));

        _tft.setTextColor(GC9A01A_WHITE);
        _tft.setCursor(cursorX - dataStart + (_getFontWidth(fontSize) * i), cursorY);
        _tft.write(_currentData[dataIndex].second.charAt(i));
      }
    }
  }
}

/// @brief Draws the data given the data center X coordinate and top Y coordinate.
/// @param dataIndex The index of the data to draw.
void DisplayHandler::_drawData(int dataIndex, FontSize fontSize, int cursorX, int cursorY)
{
  if ((unsigned int)dataIndex >= _currentData.size())
  {
    Serial.println("Data index outside of range!");
    return;
  }

  _tft.setTextSize(int(fontSize));
  _tft.setTextColor(GC9A01A_WHITE);

  _tft.setCursor(cursorX - _getCenterOffset(fontSize, _currentData[dataIndex].second.length()), cursorY);
  _tft.println(_currentData[dataIndex].second);
}

/// @brief Draws the label given the data center X coordinate and top Y coordinate.
/// @param dataIndex The index of the data to draw.
/// @param fontSize The font size to use for the text.
/// @param cursorX The X coordinate of the text cursor.
/// @param cursorY The Y coordinate of the text cursor.
void DisplayHandler::_drawLabel(int dataIndex, FontSize fontSize, int cursorX, int cursorY)
{
  if ((unsigned int)dataIndex >= _currentData.size())
  {
    Serial.println("Data index outside of range!");
    return;
  }

  _tft.setTextSize(int(fontSize));
  _tft.setTextColor(GC9A01A_WHITE);

  _tft.setCursor(cursorX - _getCenterOffset(fontSize, GaugeLabels[int(_currentData[dataIndex].first)].length()),
                 cursorY);
  _tft.println(GaugeLabels[int(_currentData[dataIndex].first)]);
}

/// @brief Draws an icon if the data at the current index is active.
/// @param dataIndex The index of the data to check for icon drawing.
/// @param bitmap The bitmap data for the icon.
/// @param iconHeight The height of the icon in pixels.
/// @param iconWidth The width of the icon in pixels.
/// @param cursorX The X coordinate to draw the icon at.
/// @param cursorY The Y coordinate to draw the icon at.
/// @param color The color to draw the icon in.
void DisplayHandler::_drawIcon(int dataIndex, const uint8_t* bitmap, int iconHeight, int iconWidth, int cursorX,
                               int cursorY, int color)
{
  if ((unsigned int)dataIndex >= _currentData.size())
  {
    Serial.println("Data index outside of range!");
    return;
  }

  if (_currentData[dataIndex].second == 1)
  {
    _tft.drawBitmap(cursorX, cursorY, bitmap, iconWidth, iconHeight, color);
  }
  else
  {
    _tft.drawBitmap(cursorX, cursorY, bitmap, iconWidth, iconHeight, GC9A01A_BLACK);
  }
}

/// @brief Returns the width in pixels of a given font size
/// @param fontSize The font size to measure
/// @return The width in pixels
int DisplayHandler::_getFontWidth(FontSize fontSize) const
{
  // Font width grows in multiples of 6
  return int(fontSize) * 6;
}

/// @brief Returns the height in pixels of a given font size
/// @param fontSize The font size to measure
/// @return The height in pixels
int DisplayHandler::_getFontHeight(FontSize fontSize) const
{
  // Font height grows in multiples of 8
  return int(fontSize) * 8;
}

/// @brief Returns the center offset in pixels for a given font size and string length
/// @param fontSize The font size to measure
/// @param length The length of the string
/// @return The center offset in pixels
int DisplayHandler::_getCenterOffset(FontSize fontSize, int length) const
{
  return (length * _getFontWidth(fontSize)) / 2;
}

/// @brief Draws the dashboard view on the TFT display.
void DisplayHandler::_drawDashboard()
{
  clearScreen();

  if (_currentData.size() < 5)
  {
    Serial.println("Current data has less than 5 gauges!");
  }

  _drawData(0, FontSize::kFontSizeLarge, _screenWidth / 2, (_screenHeight / 2) - 90);
  _drawLabel(0, FontSize::kFontSizeMedium, _screenWidth / 2, (_screenHeight / 2) - 60);

  _drawData(1, FontSize::kFontSizeLarge, _screenWidth / 4, (_screenHeight / 2) - 40);
  _drawLabel(1, FontSize::kFontSizeMedium, _screenWidth / 4, (_screenHeight / 2) - 10);

  _drawData(2, FontSize::kFontSizeLarge, _screenWidth / 4, (_screenHeight / 2) + 20);
  _drawLabel(2, FontSize::kFontSizeMedium, _screenWidth / 4, (_screenHeight / 2) + 50);

  _drawData(3, FontSize::kFontSizeLarge, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) - 40);
  _drawLabel(3, FontSize::kFontSizeMedium, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) - 10);

  _drawData(4, FontSize::kFontSizeLarge, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) + 20);
  _drawLabel(4, FontSize::kFontSizeMedium, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) + 50);

  _drawIcon(5, fan_icon, 32, 32, _screenWidth - (_screenWidth / 4) - 30, (_screenHeight / 2) + 72, GC9A01A_YELLOW);
  _drawIcon(6, cold_icon, 32, 32, (_screenWidth / 2) - 16, (_screenHeight / 2) + 80, GC9A01A_BLUE);
}

/// @brief Refreshes changed data on the dashboard view.
void DisplayHandler::_refreshDashboard()
{
  if (_dataUpdated)
  {
    if (_oldData[0].second != _currentData[0].second)
    {
      _refreshData(0, FontSize::kFontSizeLarge, _screenWidth / 2, (_screenHeight / 2) - 90);
    }
    if (_oldData[1].second != _currentData[1].second)
    {
      _refreshData(1, FontSize::kFontSizeLarge, _screenWidth / 4, (_screenHeight / 2) - 40);
    }
    if (_oldData[2].second != _currentData[2].second)
    {
      _refreshData(2, FontSize::kFontSizeLarge, _screenWidth / 4, (_screenHeight / 2) + 20);
    }
    if (_oldData[3].second != _currentData[3].second)
    {
      _refreshData(3, FontSize::kFontSizeLarge, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) - 40);
    }
    if (_oldData[4].second != _currentData[4].second)
    {
      _refreshData(4, FontSize::kFontSizeLarge, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) + 20);
    }
    if (_oldData[5].second != _currentData[5].second)
    {
      _drawIcon(5, fan_icon, 32, 32, _screenWidth - (_screenWidth / 4) - 30, (_screenHeight / 2) + 72, GC9A01A_YELLOW);
    }
    if (_oldData[6].second != _currentData[6].second)
    {
      _drawIcon(6, cold_icon, 32, 32, (_screenWidth / 2) - 16, (_screenHeight / 2) + 80, GC9A01A_BLUE);
    }
  }
}

/// @brief Draws the 4 gauge view on the TFT display.
void DisplayHandler::_drawQuad()
{
  clearScreen();

  if (_currentData.size() < 4)
  {
    Serial.println("Current data has less than 4 gauges!");
  }

  _drawLabel(0, FontSize::kFontSizeMedium, _screenWidth / 4, (_screenHeight / 2) - 20);
  _drawLabel(1, FontSize::kFontSizeMedium, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) - 20);
  _drawLabel(2, FontSize::kFontSizeMedium, _screenWidth / 4, (_screenHeight / 2) + 6);
  _drawLabel(3, FontSize::kFontSizeMedium, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) + 6);

  _drawData(0, FontSize::kFontSizeLarge, _screenWidth / 4, (_screenHeight / 2) - 50);
  _drawData(1, FontSize::kFontSizeLarge, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) - 50);
  _drawData(2, FontSize::kFontSizeLarge, _screenWidth / 4, (_screenHeight / 2) + 30);
  _drawData(3, FontSize::kFontSizeLarge, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) + 30);
}

/// @brief Refreshes changed data on the 4 gauge view.
void DisplayHandler::_refreshQuad()
{
  if (_dataUpdated)
  {
    if (_oldData[0].second != _currentData[0].second)
    {
      _refreshData(0, FontSize::kFontSizeLarge, (_screenWidth / 4), (_screenHeight / 2) - 50);
    }
    if (_oldData[1].second != _currentData[1].second)
    {
      _refreshData(1, FontSize::kFontSizeLarge, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) - 50);
    }
    if (_oldData[2].second != _currentData[2].second)
    {
      _refreshData(2, FontSize::kFontSizeLarge, (_screenWidth / 4), (_screenHeight / 2) + 30);
    }
    if (_oldData[3].second != _currentData[3].second)
    {
      _refreshData(3, FontSize::kFontSizeLarge, _screenWidth - (_screenWidth / 4), (_screenHeight / 2) + 30);
    }
  }
}

/// @brief Draws the 2 gauge view on the TFT display.
void DisplayHandler::_drawDual()
{
  clearScreen();

  if (_currentData.size() < 2)
  {
    Serial.println("Current data has less than 2 gauges!");
  }

  _drawLabel(0, FontSize::kFontSizeMedium, _screenWidth / 2, (_screenHeight / 2) - 20);
  _drawLabel(1, FontSize::kFontSizeMedium, _screenWidth / 2, (_screenHeight / 2) + 6);

  _drawData(0, FontSize::kFontSizeXL, _screenWidth / 2, (_screenHeight / 2) - 75);
  _drawData(1, FontSize::kFontSizeXL, _screenWidth / 2, (_screenHeight / 2) + 55);
}

/// @brief Refreshes changed data on the 2 gauge view.
void DisplayHandler::_refreshDual()
{
  if (_dataUpdated)
  {
    if (_oldData[0].second != _currentData[0].second)
    {
      _refreshData(0, FontSize::kFontSizeXL, (_screenWidth / 2), (_screenHeight / 2) - 75);
    }
    if (_oldData[1].second != _currentData[1].second)
    {
      _refreshData(1, FontSize::kFontSizeXL, (_screenWidth / 2), (_screenHeight / 2) + 55);
    }
  }
}

/// @brief Draws the 1 gauge view on the TFT display.
void DisplayHandler::_drawSingle()
{
  clearScreen();

  if (_currentData.size() < 1)
  {
    Serial.println("Current data has less than 1 gauge!");
  }

  _drawLabel(0, FontSize::kFontSizeLarge, _screenWidth / 2, (_screenHeight / 2) + 55);

  _drawData(0, FontSize::kFontSizeXXXL, _screenWidth / 2, (_screenHeight / 2) - 70);
}

/// @brief Refreshes changed data on the 1 gauge view.
void DisplayHandler::_refreshSingle()
{
  if (_dataUpdated)
  {
    if (_oldData[0].second != _currentData[0].second)
    {
      _refreshData(0, FontSize::kFontSizeXXXL, (_screenWidth / 2), (_screenHeight / 2) - 70);
    }
  }
}

/// @brief Highlights a label at the given position with specified text and background colors.
/// @param dataIndex The index of the data whose label is to be highlighted.
/// @param fontSize The font size to use for the label.
/// @param cursorX The X coordinate of the label's center.
/// @param cursorY The Y coordinate of the label's center.
/// @param textColor The color to use for the text.
/// @param backgroundColor The color to use for the background.
void DisplayHandler::_highlightLabel(int dataIndex, FontSize fontSize, int cursorX, int cursorY, uint16_t textColor,
                                     uint16_t backgroundColor)
{
  _tft.setTextSize(int(fontSize));
  _tft.setTextColor(textColor);

  if ((unsigned int)dataIndex >= _currentData.size())
  {
    Serial.println("Data index outside of range!");
    return;
  }

  _tft.fillRect(cursorX - _getCenterOffset(fontSize, GaugeLabels[int(_currentData[dataIndex].first)].length()) - 1,
                cursorY - 1,
                _getFontWidth(FontSize::kFontSizeMedium) * GaugeLabels[int(_currentData[dataIndex].first)].length(),
                _getFontHeight(FontSize::kFontSizeMedium), backgroundColor);
  _tft.setCursor(
      cursorX - _getCenterOffset(FontSize::kFontSizeMedium, GaugeLabels[int(_currentData[dataIndex].first)].length()),
      cursorY);
  _tft.println(GaugeLabels[int(_currentData[dataIndex].first)]);
}

/// @brief Highlights a gauge to be used as a cursor. Invert can be set to move the cursor.
/// @param textColor The color to use for the text.
/// @param backgroundColor The color to use for the background.
void DisplayHandler::_highlightQuadGauge(uint16_t textColor, uint16_t backgroundColor)
{
  switch (_gaugeCursorIndex)
  {
  case 0:
    _highlightLabel(_gaugeCursorIndex, FontSize::kFontSizeMedium, _screenWidth / 4, (_screenHeight / 2) - 20, textColor,
                    backgroundColor);
    break;
  case 1:
    _highlightLabel(_gaugeCursorIndex, FontSize::kFontSizeMedium, _screenWidth - (_screenWidth / 4),
                    (_screenHeight / 2) - 20, textColor, backgroundColor);
    break;
  case 2:
    _highlightLabel(_gaugeCursorIndex, FontSize::kFontSizeMedium, _screenWidth / 4, (_screenHeight / 2) + 6, textColor,
                    backgroundColor);
    break;
  case 3:
    _highlightLabel(_gaugeCursorIndex, FontSize::kFontSizeMedium, _screenWidth - (_screenWidth / 4),
                    (_screenHeight / 2) + 6, textColor, backgroundColor);
    break;
  case 4:
    _drawBackArrow(textColor, backgroundColor);
    break;
  }
}

/// @brief Highlights a gauge to be used as a cursor. Invert can be set to move the cursor.
/// @param textColor The color to use for the text.
/// @param backgroundColor The color to use for the background.
void DisplayHandler::_highlightDualGauge(uint16_t textColor, uint16_t backgroundColor)
{
  switch (_gaugeCursorIndex)
  {
  case 0:
    _highlightLabel(_gaugeCursorIndex, FontSize::kFontSizeMedium, _screenWidth / 2, (_screenHeight / 2) - 20, textColor,
                    backgroundColor);
    break;
  case 1:
    _highlightLabel(_gaugeCursorIndex, FontSize::kFontSizeMedium, _screenWidth / 2, (_screenHeight / 2) + 6, textColor,
                    backgroundColor);
    break;
  case 2:
    _drawBackArrow(textColor, backgroundColor);
    break;
  }
}

/// @brief Draws the back arrow on the display with specified colors.
/// @param arrowColor The color to use for the arrow.
/// @param backgroundColor The color to use for the background.
void DisplayHandler::_drawBackArrow(uint16_t arrowColor, uint16_t backgroundColor)
{
  const int kBackArrowWidth = 30;
  const int kBackArrowHeight = 19;

  _tft.drawBitmap((_screenWidth / 2) - kBackArrowWidth - 5, _screenHeight - kBackArrowHeight - 5, back, kBackArrowWidth,
                  kBackArrowHeight, arrowColor, backgroundColor);
}