#include "ui_elements.h"
#include "constants.h"

TopBarElement::TopBarElement(std::string label,
                             std::function<void(Vector2)> execute, int width)
    : label(std::move(label)), execute(std::move(execute)), width(width) {}

TopBar::TopBar() : height(50) {}

void TopBar::draw() {
  DrawRectangle(0, 0, GetScreenWidth(), height, LIGHTGRAY);
  int currentX = 10;
  for (size_t i = 0; i < elems.size(); i++) {
    // this assumes button height is 40
    elems[i].execute(
        (Vector2){static_cast<float>(currentX), (float)(height - 40) / 2});
    currentX += elems[i].width;
  }
}

SidePanelElement::SidePanelElement(std::string label,
                                   std::function<void(Vector2)> execute,
                                   int height)
    : label(std::move(label)), execute(std::move(execute)), height(height) {}

SidePanel::SidePanel() : width(SIDE_PANEL_WIDTH) {}

void SidePanel::draw(int yOffset) {
  int screenWidth = GetScreenWidth();
  DrawRectangle(screenWidth - width, yOffset, 10, GetScreenHeight() - yOffset,
                LIGHTGRAY);
  int totalHeight = 10 + yOffset;
  for (size_t i = 0; i < elems.size(); i++) {
    // Cache height before execution to avoid crash if elems is
    // modified/cleared during execute
    int elemHeight = elems[i].height;
    elems[i].execute((Vector2){static_cast<float>(screenWidth - width),
                               static_cast<float>(totalHeight)});
    if (i >= elems.size())
      break; // Stop if vector was cleared/shrank
    totalHeight += elemHeight;
  }
}
