#pragma once

#include "raylib.h"
#include <functional>
#include <string>
#include <vector>

struct AppState;

struct TopBarElement
{
	std::string label;
	int width;
	std::function<void(Vector2)> execute;
	TopBarElement(std::string label, std::function<void(Vector2)> execute, int width);
};

struct TopBar
{
	int height;
	std::vector<TopBarElement> elems;
	void draw(int screenWidth);
	TopBar();
};

struct SidePanelElement
{
	std::string label;
	int height;
	std::function<void(Vector2)> execute;
	SidePanelElement(std::string label, std::function<void(Vector2)> execute, int height);
};

struct SidePanel
{
	int width;
	Vector2 scroll;
	std::vector<SidePanelElement> elems;
	void draw(int yOffset, int screenWidth, int screenHeight);
	SidePanel();
};
