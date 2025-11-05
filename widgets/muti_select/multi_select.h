#pragma once
#include "imgui.h"
#include <vector>
#include <string>
#include <functional>

// Generic Multi-Select Dropdown Component
// Can be used for channels, filters, data sources, or any list of items
class MultiSelect {
public:
    // Constructor with number of items
    explicit MultiSelect(int numItems);
    
    // Constructor with item names
    explicit MultiSelect(const std::vector<std::string>& itemNames);
    
    // Draw the dropdown UI (returns true if selection changed)
    // label: The text shown on the button (e.g., "Channels", "Filters", "Sources")
    // Returns true if the selection changed this frame
    bool Draw(const char* label = "Select Items");
    
    // Check if a specific item is selected
    bool IsSelected(int index) const;
    
    // Get list of all selected item indices
    std::vector<int> GetSelectedIndices() const;
    
    // Get count of selected items
    int GetSelectedCount() const;
    
    // Get total number of items
    int GetItemCount() const { return m_numItems; }
    
    // Select/deselect an item
    void SetSelected(int index, bool selected);
    
    // Select all items
    void SelectAll();
    
    // Deselect all items
    void ClearAll();
    
    // Set item name
    void SetItemName(int index, const std::string& name);
    
    // Get item name
    std::string GetItemName(int index) const;
    
    // Set all item names at once
    void SetItemNames(const std::vector<std::string>& names);
    
    // Set custom button label format
    // Use %d as placeholder for count, %s for label
    // Default: "%d %s Selected"
    void SetButtonLabelFormat(const std::string& format);
    
    // Set dropdown dimensions
    void SetDropdownSize(float width, float height);
    
    // Set button dimensions
    void SetButtonSize(float width, float height);
    
    // Enable/disable "Select All" and "Clear" buttons
    void ShowControlButtons(bool show);
    
    // Set custom callback for when selection changes
    void SetOnSelectionChanged(std::function<void(const std::vector<int>&)> callback);
    
    // Set whether dropdown should auto-close when clicking outside
    void SetAutoClose(bool autoClose) { m_autoClose = autoClose; }
    
    // Manually open/close the dropdown
    void Open() { m_isOpen = true; }
    void Close() { m_isOpen = false; }
    bool IsOpen() const { return m_isOpen; }
    
private:
    int m_numItems;
    std::vector<bool> m_selected;
    std::vector<std::string> m_itemNames;
    bool m_isOpen;
    bool m_autoClose;
    bool m_showControlButtons;
    
    // UI dimensions
    float m_buttonWidth;
    float m_buttonHeight;
    float m_dropdownWidth;
    float m_dropdownHeight;
    
    // Label format
    std::string m_buttonLabelFormat;
    
    // Callback
    std::function<void(const std::vector<int>&)> m_onSelectionChanged;
    
    // Helper to generate default item name
    std::string GetDefaultItemName(int index) const;
    
    // Helper to format button label
    std::string FormatButtonLabel(const char* label, int count) const;
};