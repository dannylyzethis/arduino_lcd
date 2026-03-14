#ifndef SAM_DISPLAY_H
#define SAM_DISPLAY_H

#include "sam_config.h"

void drawDivider();
uint16_t getLines(const String& txt);
uint16_t getLinesBottom(const String& txt);
void showText(const char* txt);
void showTextBottom(const char* txt);
void applyViewMode();
void clearLinkIndicators();
void drawLinkIndicators();
void setCol(String& n);
void setBgCol(String& n);
void setBotCol(String& n);
uint16_t getColorFromName(String& n);
void parseRect(String p);
void parseCirc(String p);
void parseLine(String p);
void parseFill(String p);
void parseProg(String p);
void handleGauge(String params);
void handleBarGraph(String params);
void handleNumBox(String params);
void handleTrend(String params);
void handleXYPlot(String params);
void handleScroll(String params);

#endif // SAM_DISPLAY_H
