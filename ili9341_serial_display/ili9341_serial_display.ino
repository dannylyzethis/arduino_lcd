/*
 * ILI9341 Serial Display Controller for MCUFRIEND Shields
 * Receives data from serial port and displays on ILI9341 2.4" or 2.8" TFT Shield
 * For Arduino Uno R3
 */

#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // Library for TFT shields

// Create display object (no pins needed for shields)
MCUFRIEND_kbv tft;

// Display settings
int screenWidth = 240;
int screenHeight = 320;
#define TEXT_SIZE 2
#define TEXT_COLOR TFT_WHITE
#define BG_COLOR TFT_BLACK

// Track current text color separately
uint16_t currentTextColor = TEXT_COLOR;

// Text positioning
int cursorX = 0;
int cursorY = 0;
int lineHeight = 16; // Height of text line with size 2

// Serial communication settings
String inputString = "";
boolean stringComplete = false;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("ILI9341 Serial Display Ready");
  
  // Initialize display
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3 || ID == 0x00D3) ID = 0x9341; // Force ILI9341 for write-only shields
  tft.begin(ID);
  tft.setRotation(0); // Start with portrait
  updateScreenDimensions(); // Set initial dimensions
  tft.fillScreen(BG_COLOR);
  tft.setTextColor(currentTextColor);
  tft.setTextSize(TEXT_SIZE);
  tft.setCursor(0, 0);
  
  // Display startup message
  tft.println("Serial Display");
  tft.println("Ready...");
  cursorY = 32;
  
  // Reserve space for input string
  inputString.reserve(200);
}

void loop() {
  // Process serial data when complete command is received
  if (stringComplete) {
    processCommand(inputString);
    inputString = "";
    stringComplete = false;
  }
}

// Serial event handler - runs between loop() calls
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    // Check for newline (end of command)
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      // Add character to input string
      inputString += inChar;
    }
  }
}

// Process commands received via serial
void processCommand(String command) {
  command.trim(); // Remove any whitespace
  
  // Check for special commands (prefixed with #)
  if (command.startsWith("#")) {
    handleSpecialCommand(command);
  } else {
    // Regular text - display it
    displayText(command);
  }
}

// Handle special commands
void handleSpecialCommand(String cmd) {
  cmd.toUpperCase(); // Make case-insensitive
  
  if (cmd == "#CLEAR" || cmd == "#CLR") {
    // Clear screen
    tft.fillScreen(BG_COLOR);
    cursorX = 0;
    cursorY = 0;
    Serial.println("Screen cleared");
    
  } else if (cmd.startsWith("#COLOR ")) {
    // Change text color: #COLOR RED, #COLOR GREEN, etc.
    String color = cmd.substring(7);
    setTextColor(color);
    
  } else if (cmd.startsWith("#SIZE ")) {
    // Change text size: #SIZE 1, #SIZE 2, #SIZE 3
    int size = cmd.substring(6).toInt();
    if (size >= 1 && size <= 5) {
      tft.setTextSize(size);
      lineHeight = 8 * size; // Adjust line height
      Serial.println("Text size: " + String(size));
    }
    
  } else if (cmd.startsWith("#POS ")) {
    // Set cursor position: #POS X Y
    int spaceIndex = cmd.indexOf(' ', 5);
    if (spaceIndex > 0) {
      int x = cmd.substring(5, spaceIndex).toInt();
      int y = cmd.substring(spaceIndex + 1).toInt();
      cursorX = x;
      cursorY = y;
      tft.setCursor(cursorX, cursorY);
      Serial.println("Position: " + String(x) + "," + String(y));
    }
    
  } else if (cmd == "#INFO") {
    // Display info
    Serial.println("ILI9341 Display Info:");
    Serial.println("Resolution: " + String(screenWidth) + "x" + String(screenHeight));
    Serial.println("Current pos: " + String(cursorX) + "," + String(cursorY));
    
  } else if (cmd.startsWith("#RECT ")) {
    // Draw rectangle: #RECT X Y W H
    parseAndDrawRect(cmd.substring(6));
    
  } else if (cmd.startsWith("#CIRCLE ")) {
    // Draw circle: #CIRCLE X Y R
    parseAndDrawCircle(cmd.substring(8));
    
  } else if (cmd.startsWith("#LINE ")) {
    // Draw line: #LINE X1 Y1 X2 Y2
    parseAndDrawLine(cmd.substring(6));
    
  } else if (cmd.startsWith("#ROTATION ")) {
    // Set rotation: #ROTATION 0-3
    int rot = cmd.substring(10).toInt();
    if (rot >= 0 && rot <= 3) {
      tft.setRotation(rot);
      updateScreenDimensions();
      tft.fillScreen(BG_COLOR);
      cursorX = 0;
      cursorY = 0;
      tft.setCursor(cursorX, cursorY);
      Serial.println("Rotation set to: " + String(rot));
    }
  } else if (cmd.startsWith("#SCROLL ")) {
    // Scroll screen: #SCROLL <pixels> (positive for down)
    int pixels = cmd.substring(8).toInt();
    tft.vertScroll(0, screenHeight, pixels);
    cursorY -= pixels;  // Adjust cursor if needed
    Serial.println("Scrolled by: " + String(pixels));
    
  } else if (cmd.startsWith("#PROGRESS ")) {
    // Draw progress bar: #PROGRESS X Y W H PERCENT
    parseAndDrawProgress(cmd.substring(10));
   
    
  } else if (cmd == "#HELP") {
    printHelp();
  }
   else {
    Serial.println("Unknown command: " + cmd);
  }
}

// Display text on screen
void displayText(String text) {
  // Check if we need to wrap to next line
  if (cursorY + lineHeight > screenHeight) {
    // Scroll up by clearing and starting from top
    tft.fillScreen(BG_COLOR);
    cursorY = 0;
  }
  
  tft.setCursor(cursorX, cursorY);
  tft.println(text);
  
  // Update cursor position for next line
  cursorX = 0;
  cursorY += lineHeight;
  
  // Echo back to serial
  Serial.println("Displayed: " + text);
}

// Set text color based on color name
void setTextColor(String colorName) {
  uint16_t color = currentTextColor;
  
  colorName.toUpperCase(); // Ensure case-insensitive
  
  if (colorName == "RED") color = TFT_RED;
  else if (colorName == "GREEN") color = TFT_GREEN;
  else if (colorName == "BLUE") color = TFT_BLUE;
  else if (colorName == "YELLOW") color = TFT_YELLOW;
  else if (colorName == "CYAN") color = TFT_CYAN;
  else if (colorName == "MAGENTA") color = TFT_MAGENTA;
  else if (colorName == "WHITE") color = TFT_WHITE;
  else if (colorName == "BLACK") color = TFT_BLACK;
  else if (colorName == "ORANGE") color = TFT_ORANGE;
  else if (colorName == "PINK") color = TFT_PINK; // Use library define if available, or 0xF81F
  
  tft.setTextColor(color);
  currentTextColor = color;
  Serial.println("Color set to: " + colorName);
}

// Parse and draw rectangle
void parseAndDrawRect(String params) {
  int vals[4];
  int idx = 0;
  int lastSpace = -1;
  
  for (int i = 0; i <= params.length() && idx < 4; i++) {
    if (i == params.length() || params[i] == ' ') {
      vals[idx++] = params.substring(lastSpace + 1, i).toInt();
      lastSpace = i;
    }
  }
  
  if (idx == 4) {
    tft.drawRect(vals[0], vals[1], vals[2], vals[3], currentTextColor);
    Serial.println("Rectangle drawn");
  }
}

// Parse and draw circle
void parseAndDrawCircle(String params) {
  int vals[3];
  int idx = 0;
  int lastSpace = -1;
  
  for (int i = 0; i <= params.length() && idx < 3; i++) {
    if (i == params.length() || params[i] == ' ') {
      vals[idx++] = params.substring(lastSpace + 1, i).toInt();
      lastSpace = i;
    }
  }
  
  if (idx == 3) {
    tft.drawCircle(vals[0], vals[1], vals[2], currentTextColor);
    Serial.println("Circle drawn");
  }
}

// Parse and draw line
void parseAndDrawLine(String params) {
  int vals[4];
  int idx = 0;
  int lastSpace = -1;
  
  for (int i = 0; i <= params.length() && idx < 4; i++) {
    if (i == params.length() || params[i] == ' ') {
      vals[idx++] = params.substring(lastSpace + 1, i).toInt();
      lastSpace = i;
    }
  }
  
  if (idx == 4) {
    tft.drawLine(vals[0], vals[1], vals[2], vals[3], currentTextColor);
    Serial.println("Line drawn");
  }
}

// Parse and draw progress bar
void parseAndDrawProgress(String params) {
  int vals[5];
  int idx = 0;
  int lastSpace = -1;
  
  for (int i = 0; i <= params.length() && idx < 5; i++) {
    if (i == params.length() || params[i] == ' ') {
      vals[idx++] = params.substring(lastSpace + 1, i).toInt();
      lastSpace = i;
    }
  }
  
  if (idx == 5) {
    int percent = constrain(vals[4], 0, 100);
    int fillW = (vals[2] * percent) / 100;
    tft.fillRect(vals[0], vals[1], fillW, vals[3], currentTextColor);
    // Optional: Draw outline
    tft.drawRect(vals[0], vals[1], vals[2], vals[3], currentTextColor);
    Serial.println("Progress bar at " + String(percent) + "%");
  }
}
// Update screen dimensions based on current rotation
void updateScreenDimensions() {
  screenWidth = tft.width();
  screenHeight = tft.height();
}

// Print help information
void printHelp() {
  Serial.println("=== ILI9341 Serial Display Commands ===");
  Serial.println("Text: Just type text and press Enter");
  Serial.println("Commands (start with #):");
  Serial.println("  #CLEAR or #CLR - Clear screen");
  Serial.println("  #COLOR <name> - Set text color");
  Serial.println("    Colors: RED, GREEN, BLUE, YELLOW, CYAN,");
  Serial.println("            MAGENTA, WHITE, BLACK, ORANGE, PINK");
  Serial.println("  #SIZE <1-5> - Set text size");
  Serial.println("  #POS <x> <y> - Set cursor position");
  Serial.println("  #RECT <x> <y> <w> <h> - Draw rectangle");
  Serial.println("  #CIRCLE <x> <y> <radius> - Draw circle");
  Serial.println("  #LINE <x1> <y1> <x2> <y2> - Draw line");
  Serial.println("  #ROTATION <0-3> - Set display rotation (0=portrait, 1=landscape, etc.)");
  Serial.println("  #INFO - Display current settings");
  Serial.println("  #HELP - Show this help");
  Serial.println("=======================================");
}