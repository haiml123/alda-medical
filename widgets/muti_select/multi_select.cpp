#include "multi_select.h"
#include <cstdio>

MultiSelect::MultiSelect(int numItems)
    : m_numItems(numItems)
    , m_isOpen(false)
    , m_autoClose(true)
    , m_showControlButtons(true)
    , m_buttonWidth(200.0f)
    , m_buttonHeight(36.0f)
    , m_dropdownWidth(220.0f)
    , m_dropdownHeight(400.0f)
    , m_buttonLabelFormat("%d Items Selected")
{
    m_selected.resize(numItems, true); // All selected by default
    m_itemNames.resize(numItems);
    
    // Generate default item names
    for (int i = 0; i < numItems; ++i) {
        m_itemNames[i] = GetDefaultItemName(i);
    }
}

MultiSelect::MultiSelect(const std::vector<std::string>& itemNames)
    : m_numItems((int)itemNames.size())
    , m_isOpen(false)
    , m_autoClose(true)
    , m_showControlButtons(true)
    , m_buttonWidth(200.0f)
    , m_buttonHeight(36.0f)
    , m_dropdownWidth(220.0f)
    , m_dropdownHeight(400.0f)
    , m_buttonLabelFormat("%d Items Selected")
    , m_itemNames(itemNames)
{
    m_selected.resize(m_numItems, true); // All selected by default
}

std::string MultiSelect::GetDefaultItemName(int index) const {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "Item %d", index + 1);
    return std::string(buffer);
}

std::string MultiSelect::FormatButtonLabel(const char* label, int count) const {
    char buffer[256];
    
    // Check if format string contains both %d and %s
    if (m_buttonLabelFormat.find("%d") != std::string::npos && 
        m_buttonLabelFormat.find("%s") != std::string::npos) {
        std::snprintf(buffer, sizeof(buffer), m_buttonLabelFormat.c_str(), count, label);
    }
    // Check if format string contains only %d
    else if (m_buttonLabelFormat.find("%d") != std::string::npos) {
        std::snprintf(buffer, sizeof(buffer), m_buttonLabelFormat.c_str(), count);
    }
    // No format specifiers, use as-is
    else {
        std::snprintf(buffer, sizeof(buffer), "%s", m_buttonLabelFormat.c_str());
    }
    
    return std::string(buffer);
}

bool MultiSelect::Draw(const char* label) {
    bool selectionChanged = false;
    std::vector<int> previousSelection = GetSelectedIndices();
    
    // Count selected items
    int selectedCount = GetSelectedCount();
    
    // Create the dropdown button label
    std::string buttonLabel = FormatButtonLabel(label, selectedCount);
    
    // Store button position for dropdown positioning
    ImVec2 buttonPos = ImGui::GetCursorScreenPos();
    
    // Draw the main dropdown button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.20f, 0.23f, 1.0f));
    
    if (ImGui::Button(buttonLabel.c_str(), ImVec2(m_buttonWidth, m_buttonHeight))) {
        m_isOpen = !m_isOpen;
    }
    
    ImGui::PopStyleColor(3);
    
    // Store button rect for click detection
    ImVec2 buttonMin = ImGui::GetItemRectMin();
    ImVec2 buttonMax = ImGui::GetItemRectMax();
    
    // Draw the dropdown menu
    if (m_isOpen) {
        ImGui::SetNextWindowPos(ImVec2(buttonMin.x, buttonMax.y + 2.0f));
        ImGui::SetNextWindowSize(ImVec2(m_dropdownWidth, m_dropdownHeight));
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.18f, 0.98f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.35f, 0.40f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
        
        bool dropdownOpen = m_isOpen;
        if (ImGui::Begin("##MultiSelect", &dropdownOpen, 
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings)) {
            
            // Store dropdown window rect for click detection
            ImVec2 windowMin = ImGui::GetWindowPos();
            ImVec2 windowMax = ImVec2(windowMin.x + ImGui::GetWindowSize().x, 
                                      windowMin.y + ImGui::GetWindowSize().y);
            
            // Select All / Clear buttons
            if (m_showControlButtons) {
                float buttonWidthControl = (m_dropdownWidth - 24.0f) * 0.5f;
                
                if (ImGui::Button("Select All", ImVec2(buttonWidthControl, 30.0f))) {
                    SelectAll();
                    selectionChanged = true;
                }
                
                ImGui::SameLine();
                
                if (ImGui::Button("Clear", ImVec2(buttonWidthControl, 30.0f))) {
                    ClearAll();
                    selectionChanged = true;
                }
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
            }
            
            // Item list with checkboxes
            float listHeight = m_showControlButtons ? m_dropdownHeight - 80.0f : m_dropdownHeight - 16.0f;
            ImGui::BeginChild("##ItemList", ImVec2(0, listHeight), false);
            
            for (int i = 0; i < m_numItems; ++i) {
                bool wasSelected = m_selected[i];
                
                // Highlight selected items with a subtle background
                if (wasSelected) {
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.26f, 0.59f, 0.98f, 0.31f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.40f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.26f, 0.59f, 0.98f, 0.45f));
                    ImGui::Selectable(("##bg" + std::to_string(i)).c_str(), true, 0, ImVec2(0, 24.0f));
                    ImGui::PopStyleColor(3);
                    ImGui::SameLine(0, 0);
                }
                
                // Draw checkbox
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
                if (ImGui::Checkbox(m_itemNames[i].c_str(), &m_selected[i])) {
                    selectionChanged = true;
                }
            }
            
            ImGui::EndChild();
            
            // Auto-close detection
            if (m_autoClose && ImGui::IsMouseClicked(0)) {
                ImVec2 mousePos = ImGui::GetMousePos();
                
                // Check if click is outside both dropdown window and button
                bool outsideWindow = (mousePos.x < windowMin.x || mousePos.x > windowMax.x ||
                                      mousePos.y < windowMin.y || mousePos.y > windowMax.y);
                bool outsideButton = (mousePos.x < buttonMin.x || mousePos.x > buttonMax.x ||
                                      mousePos.y < buttonMin.y || mousePos.y > buttonMax.y);
                
                if (outsideWindow && outsideButton) {
                    m_isOpen = false;
                }
            }
        }
        ImGui::End();
        
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        
        // Handle window close button
        if (!dropdownOpen) {
            m_isOpen = false;
        }
    }
    
    // Trigger callback if selection changed
    if (selectionChanged && m_onSelectionChanged) {
        std::vector<int> currentSelection = GetSelectedIndices();
        m_onSelectionChanged(currentSelection);
    }
    
    return selectionChanged;
}

bool MultiSelect::IsSelected(int index) const {
    if (index < 0 || index >= m_numItems) {
        return false;
    }
    return m_selected[index];
}

std::vector<int> MultiSelect::GetSelectedIndices() const {
    std::vector<int> selected;
    selected.reserve(m_numItems);
    for (int i = 0; i < m_numItems; ++i) {
        if (m_selected[i]) {
            selected.push_back(i);
        }
    }
    return selected;
}

int MultiSelect::GetSelectedCount() const {
    int count = 0;
    for (bool selected : m_selected) {
        if (selected) count++;
    }
    return count;
}

void MultiSelect::SetSelected(int index, bool selected) {
    if (index >= 0 && index < m_numItems) {
        m_selected[index] = selected;
    }
}

void MultiSelect::SelectAll() {
    for (int i = 0; i < m_numItems; ++i) {
        m_selected[i] = true;
    }
}

void MultiSelect::ClearAll() {
    for (int i = 0; i < m_numItems; ++i) {
        m_selected[i] = false;
    }
}

void MultiSelect::SetItemName(int index, const std::string& name) {
    if (index >= 0 && index < m_numItems) {
        m_itemNames[index] = name;
    }
}

std::string MultiSelect::GetItemName(int index) const {
    if (index >= 0 && index < m_numItems) {
        return m_itemNames[index];
    }
    return "";
}

void MultiSelect::SetItemNames(const std::vector<std::string>& names) {
    int count = std::min((int)names.size(), m_numItems);
    for (int i = 0; i < count; ++i) {
        m_itemNames[i] = names[i];
    }
}

void MultiSelect::SetButtonLabelFormat(const std::string& format) {
    m_buttonLabelFormat = format;
}

void MultiSelect::SetDropdownSize(float width, float height) {
    m_dropdownWidth = width;
    m_dropdownHeight = height;
}

void MultiSelect::SetButtonSize(float width, float height) {
    m_buttonWidth = width;
    m_buttonHeight = height;
}

void MultiSelect::ShowControlButtons(bool show) {
    m_showControlButtons = show;
}

void MultiSelect::SetOnSelectionChanged(std::function<void(const std::vector<int>&)> callback) {
    m_onSelectionChanged = callback;
}