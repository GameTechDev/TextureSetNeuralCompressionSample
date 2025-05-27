/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Includes
#include "imgui/imgui.h"

template<typename T>
void imgui_dropdown_enum(T& currentValue, const char* label, const char* mode_labels[])
{
    const char* current_item = mode_labels[(uint32_t)currentValue];
    if (ImGui::BeginCombo(label, current_item)) // "Dropdown" is the label of the dropdown
    {
        for (int i = 0; i < (uint32_t)T::Count; i++)
        {
            bool is_selected = (current_item == mode_labels[i]); // Check if the item is selected
            if (ImGui::Selectable(mode_labels[i], is_selected))
                currentValue = (T)i;

            // Set the initial focus when opening the combo (optional)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}