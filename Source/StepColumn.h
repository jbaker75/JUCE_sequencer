#pragma once

#include <JuceHeader.h>

/**
 *  A UI component representing a single step in the sequencer.
 *  Contains a slider for pitch offset and a button for the gate.
 */
class StepColumn : public juce::Component
{
public:
    StepColumn(int stepNumber);
    ~StepColumn() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // --- State Accessors ---
    int getNoteOffset() const { return (int)pitchSlider.getValue(); }
    bool getGateState() const { return gateButton.getToggleState(); }

    void setNoteOffset(int offset) { pitchSlider.setValue(offset, juce::dontSendNotification); }
    void setGateState(bool isActive) { gateButton.setToggleState(isActive, juce::dontSendNotification); }

    /**
     *  Highlight this column when the playhead is active.
     */
    void setHighlighted(bool shouldHighlight);

    // Callbacks for when the user interacts with the UI
    std::function<void()> onValueChange;

private:
    int stepId;
    bool isHighlighted = false;

    juce::Slider pitchSlider;
    juce::ToggleButton gateButton;
    juce::Label stepLabel;
    juce::Label valueLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepColumn)
};
