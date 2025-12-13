// MenuSystem.cpp - Touch Screen Menu System Implementation

#include "MenuSystem.h"

MenuManager::MenuManager(MCUFRIEND_kbv* display) {
  tft = display;
  currentMenu = NULL;
  rootMenu = NULL;

  // Default display settings
  menuX = 0;
  menuY = 0;
  menuW = 320;
  menuH = 480;
  itemHeight = 40;
  maxVisibleItems = 10;

  // Default colors (RGB565)
  bgColor = 0x0000;        // Black
  textColor = 0xFFFF;      // White
  selectedColor = 0x07E0;  // Green
  titleColor = 0x07FF;     // Cyan
  borderColor = 0x39E7;    // Gray

  lastTouchY = -1;
  lastTouchTime = 0;
}

void MenuManager::begin(Menu* root) {
  rootMenu = root;
  currentMenu = root;

  if (currentMenu) {
    currentMenu->selectedIndex = 0;
    currentMenu->scrollOffset = 0;
  }
}

void MenuManager::setPosition(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  menuX = x;
  menuY = y;
  menuW = w;
  menuH = h;

  // Recalculate max visible items
  maxVisibleItems = (menuH - 50) / itemHeight;  // 50px for title
}

void MenuManager::setColors(uint16_t bg, uint16_t text, uint16_t selected, uint16_t title, uint16_t border) {
  bgColor = bg;
  textColor = text;
  selectedColor = selected;
  titleColor = title;
  borderColor = border;
}

void MenuManager::show() {
  if (!currentMenu || !tft) return;

  // Clear menu area
  tft->fillRect(menuX, menuY, menuW, menuH, bgColor);

  // Draw border
  tft->drawRect(menuX, menuY, menuW, menuH, borderColor);

  // Draw title bar
  tft->fillRect(menuX + 1, menuY + 1, menuW - 2, 38, 0x001F);  // Dark blue
  tft->setTextColor(titleColor, 0x001F);
  tft->setTextSize(2);
  tft->setCursor(menuX + 10, menuY + 12);
  tft->print(currentMenu->title);

  // Draw menu items
  uint16_t y = menuY + 40;
  uint8_t visibleStart = currentMenu->scrollOffset;
  uint8_t visibleEnd = min(visibleStart + maxVisibleItems, currentMenu->itemCount);

  for (uint8_t i = visibleStart; i < visibleEnd; i++) {
    bool isSelected = (i == currentMenu->selectedIndex);
    drawMenuItem(&currentMenu->items[i], y, isSelected);
    y += itemHeight;
  }

  // Draw scrollbar if needed
  if (currentMenu->itemCount > maxVisibleItems) {
    drawScrollbar();
  }
}

void MenuManager::drawMenuItem(MenuItem* item, uint16_t y, bool selected) {
  if (!item || !tft) return;

  // Background
  uint16_t bgCol = selected ? selectedColor : bgColor;
  tft->fillRect(menuX + 2, y, menuW - 4, itemHeight - 2, bgCol);

  // Text color
  uint16_t txtCol = selected ? 0x0000 : textColor;  // Black text on selected, white otherwise
  tft->setTextColor(txtCol, bgCol);
  tft->setTextSize(2);

  // Draw label
  tft->setCursor(menuX + 10, y + 12);
  tft->print(item->label);

  // Draw indicator based on type
  if (item->type == MENU_TYPE_SUBMENU) {
    // Draw arrow
    tft->setCursor(menuX + menuW - 30, y + 12);
    tft->print(">");
  } else if (item->type == MENU_TYPE_TOGGLE) {
    // Draw toggle state
    tft->setCursor(menuX + menuW - 60, y + 12);
    tft->print(item->state ? "ON" : "OFF");
  }

  // Disabled overlay
  if (!item->enabled) {
    tft->fillRect(menuX + 2, y, menuW - 4, itemHeight - 2, 0x39E7);  // Gray overlay
  }
}

void MenuManager::drawScrollbar() {
  if (!currentMenu || !tft) return;

  uint16_t scrollbarX = menuX + menuW - 10;
  uint16_t scrollbarY = menuY + 40;
  uint16_t scrollbarH = menuH - 42;

  // Draw scrollbar background
  tft->fillRect(scrollbarX, scrollbarY, 8, scrollbarH, 0x2104);  // Dark gray

  // Calculate thumb position and size
  float ratio = (float)maxVisibleItems / currentMenu->itemCount;
  uint16_t thumbH = scrollbarH * ratio;
  if (thumbH < 20) thumbH = 20;

  float scrollRatio = (float)currentMenu->scrollOffset / (currentMenu->itemCount - maxVisibleItems);
  uint16_t thumbY = scrollbarY + (scrollbarH - thumbH) * scrollRatio;

  // Draw thumb
  tft->fillRect(scrollbarX, thumbY, 8, thumbH, 0x7BEF);  // Light gray
}

int MenuManager::getTouchedItemIndex(int touchX, int touchY) {
  if (!currentMenu) return -1;

  // Check if touch is within menu bounds
  if (touchX < menuX || touchX > menuX + menuW) return -1;
  if (touchY < menuY + 40 || touchY > menuY + menuH) return -1;

  // Calculate item index
  int relY = touchY - (menuY + 40);
  int itemIdx = relY / itemHeight + currentMenu->scrollOffset;

  if (itemIdx >= 0 && itemIdx < currentMenu->itemCount) {
    return itemIdx;
  }

  return -1;
}

bool MenuManager::handleTouch(int touchX, int touchY) {
  int itemIdx = getTouchedItemIndex(touchX, touchY);

  if (itemIdx >= 0) {
    // Update selection
    currentMenu->selectedIndex = itemIdx;
    show();

    // Small delay to show selection
    delay(100);

    // Activate item
    activateSelected();
    return true;
  }

  return false;
}

void MenuManager::navigateTo(Menu* menu) {
  if (!menu) return;

  currentMenu = menu;
  currentMenu->selectedIndex = 0;
  currentMenu->scrollOffset = 0;
  show();
}

void MenuManager::selectNext() {
  if (!currentMenu) return;

  if (currentMenu->selectedIndex < currentMenu->itemCount - 1) {
    currentMenu->selectedIndex++;

    // Scroll if needed
    if (currentMenu->selectedIndex >= currentMenu->scrollOffset + maxVisibleItems) {
      currentMenu->scrollOffset++;
    }

    show();
  }
}

void MenuManager::selectPrev() {
  if (!currentMenu) return;

  if (currentMenu->selectedIndex > 0) {
    currentMenu->selectedIndex--;

    // Scroll if needed
    if (currentMenu->selectedIndex < currentMenu->scrollOffset) {
      currentMenu->scrollOffset--;
    }

    show();
  }
}

void MenuManager::activateSelected() {
  if (!currentMenu) return;

  MenuItem* item = getSelectedItem();
  if (!item || !item->enabled) return;

  // Handle based on type
  switch (item->type) {
    case MENU_TYPE_SUBMENU:
      if (item->submenu) {
        navigateTo(item->submenu);
      }
      break;

    case MENU_TYPE_ACTION:
      if (item->callback) {
        item->callback(item);
      }
      break;

    case MENU_TYPE_TOGGLE:
      item->state = !item->state;
      if (item->callback) {
        item->callback(item);
      }
      show();
      break;

    case MENU_TYPE_VALUE:
      if (item->callback) {
        item->callback(item);
      }
      break;
  }
}

void MenuManager::goBack() {
  if (currentMenu && currentMenu->parent) {
    navigateTo(currentMenu->parent);
  }
}

MenuItem* MenuManager::getSelectedItem() {
  if (!currentMenu || currentMenu->selectedIndex >= currentMenu->itemCount) {
    return NULL;
  }
  return &currentMenu->items[currentMenu->selectedIndex];
}

// Helper function to create menus
Menu* createMenu(const char* title, MenuItem* items, uint8_t count, Menu* parent) {
  Menu* menu = new Menu;
  menu->title = title;
  menu->items = items;
  menu->itemCount = count;
  menu->parent = parent;
  menu->selectedIndex = 0;
  menu->scrollOffset = 0;
  return menu;
}
