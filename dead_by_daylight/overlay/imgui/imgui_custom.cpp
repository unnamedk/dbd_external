#include "imgui_custom.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

bool ImGui::SelectableFlags(const char* label, int* flags, unsigned int flags_value, ImGuiSelectableFlags selectable_flags, const ImVec2& size)
{
	bool v = ((*flags & flags_value) == flags_value);
	bool pressed = Selectable(label, &v, selectable_flags, size);

	if (pressed) {
		if (v) {
			*flags |= flags_value;
		}
		else {
			*flags &= ~flags_value;
		}
	}

	return pressed;
}

bool ImGui::CheckboxFlags(const char* label, int* flags, unsigned int flags_value)
{
	bool v = ((*flags & flags_value) == flags_value);
	bool pressed = Checkbox(label, &v);
	if (pressed) {
		if (v)
			* flags |= flags_value;
		else
			*flags &= ~flags_value;
	}

	return pressed;
}

bool ImGui::ColorEdit4(const char* label, uint8_t col[4], ImGuiColorEditFlags flags)
{
	float temp[4] = { 0.f };
	temp[0] = static_cast<int>(col[0]) / 255.f;
	temp[1] = static_cast<int>(col[1]) / 255.f;
	temp[2] = static_cast<int>(col[2]) / 255.f;
	temp[3] = static_cast<int>(col[3]) / 255.f;

	auto result = ColorEdit4(label, temp, flags);

	col[0] = static_cast<int>(temp[0] * 255) & 0xFF;
	col[1] = static_cast<int>(temp[1] * 255) & 0xFF;
	col[2] = static_cast<int>(temp[2] * 255) & 0xFF;
	col[3] = static_cast<int>(temp[3] * 255) & 0xFF;

	return result;
}

bool ImGui::ColorPicker4( const char *label, uint8_t col[ 4 ], ImGuiColorEditFlags flags )
{
    float temp[ 4 ] = { 0.f };
    temp[ 0 ] = static_cast<int>( col[ 0 ] ) / 255.f;
    temp[ 1 ] = static_cast<int>( col[ 1 ] ) / 255.f;
    temp[ 2 ] = static_cast<int>( col[ 2 ] ) / 255.f;
    temp[ 3 ] = static_cast<int>( col[ 3 ] ) / 255.f;

    auto result = ColorPicker4( label, temp, flags, nullptr );

    col[ 0 ] = static_cast<int>( temp[ 0 ] * 255 ) & 0xFF;
    col[ 1 ] = static_cast<int>( temp[ 1 ] * 255 ) & 0xFF;
    col[ 2 ] = static_cast<int>( temp[ 2 ] * 255 ) & 0xFF;
    col[ 3 ] = static_cast<int>( temp[ 3 ] * 255 ) & 0xFF;

    return result;
}

bool ImGui::SelectKey(const char* label, int* item, const char** arr, int size)
{
	return false;
}

bool ImGui::SliderIntClamped(const char* label, int* v, int v_min, int v_max, const char* display_format)
{
	const auto ret = SliderInt(label, v, v_min, v_max, display_format);
	if (ret && (v_min < v_max)) {
		*v = ImClamp(*v, v_min, v_max);
	}

	return ret;
}

bool ImGui::SliderFloatClamped(const char* label, float* v, float v_min, float v_max, const char* display_format, float power)
{
	const auto ret = SliderFloat(label, v, v_min, v_max, display_format, power);
	if (ret && (v_min < v_max)) {
		*v = ImClamp(*v, v_min, v_max);
	}

	return ret;
}