#define IMGUI_DEFINE_MATH_OPERATORS

#ifndef IMGUI_EXTENSIONS_H
#define IMGUI_EXTENSIONS_H

#include <string>
#include <vector>
#include "imgui.h"
#include "imgui_internal.h"

namespace ImGui {

    struct UsageItem {
        std::string name;
        float       value;
        uint        color;
    };

    typedef std::vector<UsageItem> UsageList;

    static void UsageBar(UsageList usage_list , float total, const ImVec2& size_arg, const char* overlay) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), g.FontSize + style.FramePadding.y * 2.0f);
        ImRect bb(pos, pos + size);
        ItemSize(size, style.FramePadding.y);
        if (!ItemAdd(bb, 0)) return;


        // Render
        RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
        bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));

        float accum = 0.0;
        ImVec2 fill_br;
        for(auto& item : usage_list) {
            float frac = item.value / total;
            float prev = accum;
            accum += frac;
            if (accum > 1) accum = 1;
            fill_br = ImVec2(ImLerp(bb.Min.x, bb.Max.x, frac), bb.Max.y);
            RenderRectFilledRangeH(window->DrawList, bb, item.color, prev, accum, style.FrameRounding);
        }
        fill_br = ImVec2(ImLerp(bb.Min.x, bb.Max.x, accum), bb.Max.y);

        // Default displaying the fraction as percentage string, but user can override it
        char overlay_buf[32];
        if (!overlay) {
            ImFormatString(overlay_buf, IM_ARRAYSIZE(overlay_buf), "%.0f%%", accum * 100 + 0.01f);
            overlay = overlay_buf;
        }

        ImVec2 overlay_size = CalcTextSize(overlay, NULL);
        if (overlay_size.x > 0.0f) {
            RenderTextClipped(ImVec2(ImClamp(fill_br.x + style.ItemSpacing.x, bb.Min.x, bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x), bb.Min.y), bb.Max, overlay, NULL, &overlay_size, ImVec2(0.0f, 0.5f), &bb);
        }

        // -- Show Legend when mouse hovered --
        if(IsItemHovered()) {
            ImVec2 square(10,10);
            BeginTooltip();
            for(auto& item : usage_list) {
                ImVec2 p = GetCursorScreenPos()+ImVec2(0,2);
                GetWindowDrawList()->AddRectFilled(p, p+square, item.color);
                ImGui::Dummy(square);
                ImGui::SameLine();
                TextUnformatted(item.name.c_str());
            }
            EndTooltip();
        }
        //------------------------------------
    }

}  //namespace

#endif
