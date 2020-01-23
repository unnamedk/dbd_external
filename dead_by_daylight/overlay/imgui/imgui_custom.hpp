#pragma once

#include "imgui.h"
#include <cstdint>

namespace ImGui
{
	bool SelectableFlags(const char* label, int* flags, unsigned int flags_value, ImGuiSelectableFlags selectable_flags = 0, const ImVec2& size = ImVec2(0.0f, 0.0f));
	bool CheckboxFlags(const char* label, int* flags, unsigned int flags_value);
	bool ColorEdit4(const char* label, uint8_t col[4], ImGuiColorEditFlags flags);
	bool ColorPicker4(const char* label, uint8_t col[4], ImGuiColorEditFlags flags);
	bool SelectKey(const char* label, int* item, const char** arr, int size);
	bool SliderIntClamped(const char* label, int* v, int v_min, int v_max, const char* display_format = "%.0f");
	bool SliderFloatClamped(const char* label, float* v, float v_min, float v_max, const char* display_format = "%.3f", float power = 1.0f);
    void BeginGroupPanel( const char *name, const ImVec2 &size = ImVec2( -1.0f, -1.0f ) );
    void EndGroupPanel();
}