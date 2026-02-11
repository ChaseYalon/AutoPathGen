#include "ui_elements.h"
#include "constants.h"
#include "../extern/raygui/src/raygui.h"
#include <cstdio>

TopBarElement::TopBarElement(
	std::string label, std::function<void(Vector2)> execute, int width
)
	: label(std::move(label)), execute(std::move(execute)), width(width) {
}

TopBar::TopBar() : height(50) {
}

void TopBar::draw(int screenWidth) {
	DrawRectangle(0, 0, screenWidth, height, LIGHTGRAY);
	int currentX = 10;
	for (size_t i = 0; i < elems.size(); i++) {
		// this assumes button height is 40
		elems[i].execute((Vector2) {static_cast<float>(currentX),
									(float) (height - 40) / 2});
		currentX += elems[i].width;
	}
}

SidePanelElement::SidePanelElement(
	std::string label, std::function<void(Vector2)> execute, int height
)
	: label(std::move(label)), execute(std::move(execute)), height(height) {
}

SidePanel::SidePanel() : width(SIDE_PANEL_WIDTH), scroll({0, 0}) {
}

void SidePanel::draw(int yOffset, int screenWidth, int screenHeight) {
	Rectangle panelRect =
		{(float) screenWidth - width,
		 (float) yOffset,
		 (float) width,
		 (float) screenHeight - yOffset};

	// Calculate Content Height
	int contentHeight = 10;
	for (const auto& elem : elems) {
		contentHeight += elem.height;
	}
	// Add some padding at bottom
	contentHeight += 50;

	Rectangle contentRect = {0, 0, (float) width - 20, (float) contentHeight};
	Rectangle view = {0};

	// Match background color to LIGHTGRAY
	int prevBg = GuiGetStyle(DEFAULT, BACKGROUND_COLOR);
	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(LIGHTGRAY));

	GuiScrollPanel(panelRect, NULL, contentRect, &scroll, &view);

	GuiSetStyle(DEFAULT, BACKGROUND_COLOR, prevBg);

	BeginScissorMode((int) view.x, (int) view.y, (int) view.width, (int) view.height);

	int currentY = (int) view.y + (int) scroll.y + 10;	// Start with padding

	// We need to adjust drawing x position based on view.x
	// But our execute functions expect an absolute screen position usually?
	// SidePanelElement::execute takes topLeft Vector2.

	float drawX = view.x;

	for (size_t i = 0; i < elems.size(); i++) {
		// Cache height
		int elemHeight = elems[i].height;

		// Only draw if visible (culling optimization)
		if (currentY + elemHeight > view.y && currentY < view.y + view.height) {
			elems[i].execute((Vector2) {drawX, (float) currentY});
		}

		if (i >= elems.size())
			break;	// Safety
		currentY += elemHeight;
	}

	EndScissorMode();
}
