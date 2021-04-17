#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class AntagonizerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AntagonizerAudioProcessorEditor (AntagonizerAudioProcessor&);
    ~AntagonizerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Slider mDryWetSlider;
    Slider mFeedbackSlider;
    Slider mDepthSlider;
    Slider mRateSlider;
    Slider mPhaseOffsetSlider;
    
    ComboBox mTypeBox;
    
    AntagonizerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AntagonizerAudioProcessorEditor)
};
