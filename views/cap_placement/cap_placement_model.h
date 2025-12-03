#pragma once

#include <string>
#include <vector>

namespace elda::views::cap_placement {

struct PlacementStep {
    std::string title;
    std::string description;
    std::string image_path;  // Optional: path to illustration
};

class CapPlacementModel {
public:
    CapPlacementModel() {
        setup_steps();
    }

    const std::vector<PlacementStep>& steps() const { return steps_; }
    size_t step_count() const { return steps_.size(); }
    
    const PlacementStep& current_step() const { return steps_[current_step_index_]; }
    size_t current_index() const { return current_step_index_; }
    
    bool is_first_step() const { return current_step_index_ == 0; }
    bool is_last_step() const { return current_step_index_ == steps_.size() - 1; }
    
    void next_step() {
        if (!is_last_step()) {
            current_step_index_++;
        }
    }
    
    void previous_step() {
        if (!is_first_step()) {
            current_step_index_--;
        }
    }
    
    void reset() {
        current_step_index_ = 0;
    }

private:
    void setup_steps() {
        steps_ = {
            {
                "Measure Head Size",
                "Use a measuring tape to measure the circumference of the subject's head.\n"
                "Measure from the forehead (above the eyebrows) around to the back of the head.\n"
                "Select the appropriate cap size based on the measurement.",
                ""
            },
            {
                "Position the Cap",
                "Place the cap on the subject's head with the Cz electrode at the vertex.\n"
                "The vertex is located at the intersection of the midline (nasion to inion)\n"
                "and the line connecting the two preauricular points.",
                ""
            },
            {
                "Align Frontal Electrodes",
                "Ensure the Fpz electrode is positioned 10% of the nasion-inion distance\n"
                "above the nasion (bridge of the nose).\n"
                "The cap should be symmetrical on both sides.",
                ""
            },
            {
                "Secure the Cap",
                "Fasten the chin strap to secure the cap in place.\n"
                "The cap should be snug but comfortable.\n"
                "Check that no electrodes have shifted during fastening.",
                ""
            },
            {
                "Apply Gel to Electrodes",
                "Use a blunt syringe to apply conductive gel to each electrode.\n"
                "Gently abrade the scalp under each electrode using circular motions.\n"
                "This reduces impedance and improves signal quality.",
                ""
            },
            {
                "Check Impedances",
                "After gel application, check electrode impedances.\n"
                "All electrodes should show impedance below 50 kΩ for MRI safety.\n"
                "Re-apply gel to any electrodes with high impedance.",
                ""
            }
        };
    }

    std::vector<PlacementStep> steps_;
    size_t current_step_index_ = 0;
};

} // namespace elda::views::cap_placement
