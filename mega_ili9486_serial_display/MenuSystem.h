// MenuSystem.h - Modular Touch Screen Menu System
// Provides hierarchical menu navigation with FPGA integration

#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

// Menu item types
enum MenuItemType {
  MENU_TYPE_SUBMENU,     // Opens another menu
  MENU_TYPE_ACTION,      // Executes an action
  MENU_TYPE_TOGGLE,      // Toggle on/off
  MENU_TYPE_VALUE        // Shows/edits a value
};

// Menu item action types for FPGA
enum MenuAction {
  ACTION_NONE,
  ACTION_CONTROL_REG,    // Read/write control register
  ACTION_STATUS_REG,     // Read status register
  ACTION_TEMPERATURE,    // Read temperature sensors
  ACTION_FW_INFO,        // Read firmware info
  ACTION_CUSTOM_BYTES,   // Send custom byte sequence
  ACTION_BACK            // Go back to parent menu
};

// Forward declarations
struct MenuItem;
struct Menu;

// Menu item callback function type
typedef void (*MenuActionCallback)(MenuItem* item);

// Menu item structure
struct MenuItem {
  const char* label;           // Display text
  MenuItemType type;           // Item type
  MenuAction action;           // Associated action
  Menu* submenu;              // Pointer to submenu (if type == SUBMENU)
  MenuActionCallback callback; // Function to call when activated
  uint8_t* data;              // Associated data (e.g., bytes to send)
  uint8_t dataLen;            // Length of data
  bool enabled;               // Is item selectable
  bool state;                 // For toggle items
};

// Menu structure
struct Menu {
  const char* title;          // Menu title
  MenuItem* items;            // Array of menu items
  uint8_t itemCount;          // Number of items
  Menu* parent;               // Parent menu (NULL for root)
  uint8_t selectedIndex;      // Currently selected item
  uint8_t scrollOffset;       // For scrolling long menus
};

// Menu Manager Class
class MenuManager {
private:
  MCUFRIEND_kbv* tft;         // Display reference
  Menu* currentMenu;          // Current active menu
  Menu* rootMenu;             // Root menu

  // Display settings
  uint16_t menuX, menuY;      // Menu position
  uint16_t menuW, menuH;      // Menu dimensions
  uint16_t itemHeight;        // Height of each menu item
  uint16_t maxVisibleItems;   // Max items visible at once

  // Colors
  uint16_t bgColor;
  uint16_t textColor;
  uint16_t selectedColor;
  uint16_t titleColor;
  uint16_t borderColor;

  // Touch state
  int lastTouchY;
  unsigned long lastTouchTime;

  // Helper functions
  void drawMenuItem(MenuItem* item, uint16_t y, bool selected);
  void drawScrollbar();
  int getTouchedItemIndex(int touchX, int touchY);

public:
  MenuManager(MCUFRIEND_kbv* display);

  // Setup
  void begin(Menu* root);
  void setPosition(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  void setColors(uint16_t bg, uint16_t text, uint16_t selected, uint16_t title, uint16_t border);

  // Navigation
  void show();                 // Draw current menu
  void navigateTo(Menu* menu); // Switch to specific menu
  void selectNext();           // Move selection down
  void selectPrev();           // Move selection up
  void activateSelected();     // Activate currently selected item
  void goBack();              // Return to parent menu

  // Touch handling
  bool handleTouch(int touchX, int touchY);

  // Utility
  Menu* getCurrentMenu() { return currentMenu; }
  MenuItem* getSelectedItem();
  void refresh() { show(); }   // Redraw menu
};

// Helper function to create menus easily
Menu* createMenu(const char* title, MenuItem* items, uint8_t count, Menu* parent = NULL);

#endif // MENU_SYSTEM_H
