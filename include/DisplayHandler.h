#pragma once

#include "MegaSquirtInfo.h"
#include "ProgramMemory.h"

#include <Adafruit_GC9A01A.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <unordered_map>
#include <utility>
#include <vector>

enum class FontSize : int
{
  kFontSizeSmall = 1,  // Font size 6x8
  kFontSizeMedium = 2, // Font size 12x16
  kFontSizeLarge = 3,  // Font size 18x24
  kFontSizeXL = 4,     // Font size 24x36
  kFontSizeXXL = 5,    // Font size 30x44
  kFontSizeXXXL = 6,   // Font size 36x52
};

/// @brief Enum representing different gauge views.
enum class GaugeView : int
{
  kDashboard = 0,
  kQuadGauge = 1,
  kDualGauge = 2,
  kSingleGauge = 3,
  kGaugeMin = kDashboard,
  kGaugeMax = kSingleGauge,
};

struct GaugeInfo
{
  std::vector<GaugeData> gauges;
};

class DisplayHandler
{
public:
  DisplayHandler() = delete;
  DisplayHandler(int TFT_RST, int TFT_DC, int TFT_CS, int screenHeight, int screenWidth);

  void displayStartupScreen();
  void display();
  void clearScreen();

  void createBackArrow();
  void clearBackArrow();

  int getCurrentGaugeCursorIndex();
  void moveGaugeCursor(int gaugeIndex);
  void clearGaugeCursor();

  void setCurrentData(std::vector<std::pair<GaugeData, String>> newData);
  std::vector<std::pair<GaugeData, String>> getCurrentData();

  void setCurrentView(GaugeView newGauge);
  GaugeView getCurrentView();

  void setCurrentDashboardGauges(std::vector<GaugeData> gauges);
  std::vector<GaugeData> getCurrentDashboardGauges();

  void setCurrentQuadGauges(std::vector<GaugeData> gauges);
  std::vector<GaugeData> getCurrentQuadGauges();

  void setCurrentDualGauges(std::vector<GaugeData> gauges);
  std::vector<GaugeData> getCurrentDualGauges();

  void setCurrentSingleGauge(GaugeData gauges);
  GaugeData getCurrentSingleGauge();

private:
  const int _screenHeight;
  const int _screenWidth;

  Adafruit_GC9A01A _tft;

  std::unordered_map<GaugeView, GaugeInfo> _gaugeMap;

  GaugeView _currentGaugeView;
  bool _gaugeViewUpdated;

  std::vector<GaugeData> _currentDashboardGauges;
  std::vector<GaugeData> _currentQuadGauges;
  std::vector<GaugeData> _currentDualGauges;
  GaugeData _currentSingleGauge;

  std::vector<std::pair<GaugeData, String>> _currentData;
  std::vector<std::pair<GaugeData, String>> _oldData;
  bool _dataUpdated;
  int _gaugeCursorIndex;

  void _refreshData(int dataIndex, FontSize fontSize, int cursorX, int cursorY);
  void _drawData(int dataIndex, FontSize fontSize, int cursorX, int cursorY);
  void _drawLabel(int dataIndex, FontSize fontSize, int cursorX, int cursorY);
  void _highlightLabel(int dataIndex, FontSize fontSize, int cursorX, int cursorY, uint16_t textColor,
                       uint16_t backgroundColor);
  void _drawIcon(int dataIndex, const uint8_t* bitmap, int iconHeight, int iconWidth, int cursorX, int cursorY,
                 int color);
  int _getFontWidth(FontSize fontSize) const;
  int _getFontHeight(FontSize fontSize) const;
  int _getCenterOffset(FontSize fontSize, int length) const;

  void _drawDashboard();
  void _refreshDashboard();

  void _drawQuad();
  void _refreshQuad();

  void _drawDual();
  void _refreshDual();

  void _drawSingle();
  void _refreshSingle();

  void _highlightQuadGauge(uint16_t textColor, uint16_t backgroundColor);
  void _highlightDualGauge(uint16_t textColor, uint16_t backgroundColor);
  void _highlightSingleGauge(uint16_t textColor, uint16_t backgroundColor);

  void _drawBackArrow(uint16_t arrowColor, uint16_t backgroundColor);
};