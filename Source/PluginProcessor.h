#pragma once

#include <JuceHeader.h>

class AntagonizerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AntagonizerAudioProcessor();
    ~AntagonizerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    float linearInterpolation(float sample_x, float sample_x1, float inPhase);
    
private:
    int MAX_DELAY_TIME {2};
        
        AudioParameterFloat *mDryWetParameter;
        AudioParameterFloat *mFeedbackParameter;
        AudioParameterFloat *mDepthParameter;
        AudioParameterFloat *mRateParameter;
        AudioParameterFloat *mPhaseOffsetParameter;
        
        AudioParameterInt *mTypeParameter;
        
        float mLFOPhase;
            
        float mFeedbackLeft;
        float mFeedbackRight;
        
        int mCircularBufferLength;
        int mCircularBufferWriteHead;
        
        float *mCircularBufferLeft;
        float *mCircularBufferRight;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AntagonizerAudioProcessor)
};
