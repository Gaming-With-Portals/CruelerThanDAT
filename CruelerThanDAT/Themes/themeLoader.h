#pragma once
#include "vector"
#include <string>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;


struct CTDTheme {
	std::string name;
	std::string author;

	float alpha = 1.0f;
	float disabledAlpha = 1.0f;
	float windowPadding[2] = {0.8f, 0.8f};
	float windowRounding = 0.0f;
	float windowBorderSize = 0.0f;
	float windowMinSize[2] = { 32.0f, 32.0f };
	float windowTitleAlign[2] = { 0.0, 0.5f };
	std::string windowMenuButtonPosition = "Left";
	float childRounding = 0.0f;
	float childBorderSize = 1.0f;
	float popupRounding = 0.0f;
	float popupBorderSize = 1.0f;
	float framePadding[2] = { 4.0f, 3.0f };
	float frameRounding = 0.0f;
	float frameBorderSize = 1.0f;
	float itemSpacing[2] = { 8.0, 4.0 };
	float itemInnerSpacing[2] = { 4.0, 4.0 };
	float cellPadding[2] = { 4.0, 2.0 };
	float indentSpacing = 21.0f;
	float columnsMinSpacing = 6.0f;
	float scrollbarSize = 14.0f;
	float scrollbarRounding = 0.0f;
	float grabMinSize = 10.0f;
	float grabRounding = 0.0f;
	float tabRounding = 0.0f;
	float tabBorderSize = 0.0f;
	float tagMinWidthForCloseButton = 0.0f;
	std::string colorButtonPosition = "Right";
	float buttonTextAlign[2] = { 0.5f, 0.5f };
	float selectableTextAlign[2] = { 0.0f, 0.0f };



};


class ThemeManager {
	
public:

	void UpdateThemeList();

	void ChooseStyle(int id);

private:
	std::vector<CTDTheme*> themes;

};

