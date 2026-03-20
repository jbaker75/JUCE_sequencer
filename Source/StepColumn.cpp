#include "StepColumn.h"

StepColumn::StepColumn(int stepNumber) : stepId(stepNumber)
{
    // Pitch Slider Setup
    addAndMakeVisible(pitchSlider);
    pitchSlider.setSliderStyle(juce::Slider::LinearVertical);
    pitchSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pitchSlider.setRange(-12, 12, 1);
    pitchSlider.setValue(0);
    pitchSlider.onValueChange = [this] {
        valueLabel.setText(juce::String((int)pitchSlider.getValue()), juce::dontSendNotification);
        if (onValueChange) onValueChange();
    };

    // Value Label (Shows +3, -2, etc.)
    addAndMakeVisible(valueLabel);
    valueLabel.setText("0", juce::dontSendNotification);
    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setFont(juce::FontOptions(12.0f));

    // Gate Toggle Button
    addAndMakeVisible(gateButton);
    gateButton.setButtonText("");
    gateButton.setClickingTogglesState(true);
    gateButton.setColour(juce::ToggleButton::tickColourId, juce::Colours::limegreen);
    gateButton.onClick = [this] { if (onValueChange) onValueChange(); };

    // Step Number Label
    addAndMakeVisible(stepLabel);
    stepLabel.setText(juce::String(stepNumber + 1), juce::dontSendNotification);
    stepLabel.setJustificationType(juce::Justification::centred);
    stepLabel.setFont(juce::FontOptions(10.0f));
}

StepColumn::~StepColumn() {}

void StepColumn::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(2);
    
    // Draw background highlight if active
    if (isHighlighted)
    {
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
        
        g.setColour(juce::Colours::limegreen.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 2.0f);
    }
    else
    {
        g.setColour(juce::Colours::black.withAlpha(0.2f));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    }
}

void StepColumn::resized()
{
    auto area = getLocalBounds().reduced(4);
    
    // Layout: Value (Top) -> Slider (Middle) -> Button (Bottom) -> Step Num (Very Bottom)
    valueLabel.setBounds(area.removeFromTop(20));
    stepLabel.setBounds(area.removeFromBottom(15));
    gateButton.setBounds(area.removeFromBottom(30).withSizeKeepingCentre(24, 24));
    pitchSlider.setBounds(area);
}

void StepColumn::setHighlighted(bool shouldHighlight)
{
    if (isHighlighted != shouldHighlight)
    {
        isHighlighted = shouldHighlight;
        repaint();
    }
}
