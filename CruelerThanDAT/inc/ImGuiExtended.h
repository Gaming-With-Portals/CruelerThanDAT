#pragma once
#include "pch.hpp"

#include <imgui.h>
#include <imgui_internal.h>

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// https://github.com/ocornut/imgui/issues/7660
static bool HyperLink(const char* label, bool underlineWhenHoveredOnly = false)
{
    const ImU32 linkColor = ImGui::ColorConvertFloat4ToU32({ 0.2, 0.3, 0.8, 1 });
    const ImU32 linkHoverColor = ImGui::ColorConvertFloat4ToU32({ 0.4, 0.6, 0.8, 1 });
    const ImU32 linkFocusColor = ImGui::ColorConvertFloat4ToU32({ 0.6, 0.4, 0.8, 1 });

    const ImGuiID id = ImGui::GetID(label);

    ImGuiWindow* const window = ImGui::GetCurrentWindow();
    ImDrawList* const draw = ImGui::GetWindowDrawList();

    const ImVec2 pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
    const ImVec2 size = ImGui::CalcTextSize(label);
    ImRect bb(pos, { pos.x + size.x, pos.y + size.y });

    ImGui::ItemSize(bb, 0.0f);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool isHovered = false;
    const bool isClicked = ImGui::ButtonBehavior(bb, id, &isHovered, nullptr);
    const bool isFocused = ImGui::IsItemFocused();

    const ImU32 color = isHovered ? linkHoverColor : isFocused ? linkFocusColor : linkColor;

    draw->AddText(bb.Min, color, label);

    if (isFocused)
        draw->AddRect(bb.Min, bb.Max, color);
    else if (!underlineWhenHoveredOnly || isHovered)
        draw->AddLine({ bb.Min.x, bb.Max.y }, bb.Max, color);

    return isClicked;
}