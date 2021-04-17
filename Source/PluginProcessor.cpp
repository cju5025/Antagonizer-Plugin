#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AntagonizerAudioProcessor::AntagonizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(mDryWetParameter = new AudioParameterFloat("drywet", "Dry / Wet", 0.f, 1.f, 0.5f));
    
    addParameter(mFeedbackParameter = new AudioParameterFloat("feedback", "Feedback", 0.f, 1.f, 0.5f));
    
    addParameter(mDepthParameter = new AudioParameterFloat("depth", "Depth", 0.0, 1.f, 0.5f));
    
    addParameter(mRateParameter = new AudioParameterFloat("rate", "Rate", 0.1f, 20.f, 10.f));
    
    addParameter(mPhaseOffsetParameter = new AudioParameterFloat("phaseoffset", "Phase Offset", 0.f, 1.f, 0.f));
    
    addParameter(mTypeParameter = new AudioParameterInt("type", "Type", 0, 3, 0));
     
     
     
     mCircularBufferLeft = nullptr;
     mCircularBufferRight = nullptr;
     mCircularBufferWriteHead = 0;
     mCircularBufferLength = 0;
     
     mFeedbackLeft = 0;
     mFeedbackRight = 0;
    
     mLFOPhase = 0;
}

AntagonizerAudioProcessor::~AntagonizerAudioProcessor()
{
    if (mCircularBufferLeft != nullptr)
        {
            delete [] mCircularBufferLeft;
            mCircularBufferLeft = nullptr;
        }
        
        if (mCircularBufferRight != nullptr)
        {
            delete [] mCircularBufferRight;
            mCircularBufferRight = nullptr;
            
        }
}

//==============================================================================
const juce::String AntagonizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AntagonizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AntagonizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AntagonizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AntagonizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AntagonizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AntagonizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AntagonizerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AntagonizerAudioProcessor::getProgramName (int index)
{
    return {};
}

void AntagonizerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AntagonizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mLFOPhase = 0;
       
           mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
           
           if (mCircularBufferLeft == nullptr)
           {
               mCircularBufferLeft = new float[mCircularBufferLength];
           }
           
           zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));
           
           if (mCircularBufferRight == nullptr)
           {
               mCircularBufferRight = new float[mCircularBufferLength];
           }
           
           zeromem(mCircularBufferRight, mCircularBufferLength * sizeof(float));
           
           mCircularBufferWriteHead = 0;
}

void AntagonizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AntagonizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AntagonizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
            juce::ScopedNoDenormals noDenormals;
            auto totalNumInputChannels  = getTotalNumInputChannels();
            auto totalNumOutputChannels = getTotalNumOutputChannels();


            for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
                buffer.clear (i, 0, buffer.getNumSamples());
                
            float* leftChannel = buffer.getWritePointer(0);
            float* rightChannel = buffer.getWritePointer(1);

            for (int i = 0; i < buffer.getNumSamples(); i++)
            {
                mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
                mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;
                
                float lfoOutLeft = sin(2*M_PI * mLFOPhase);
                
                float lfoPhaseRight = mLFOPhase += *mPhaseOffsetParameter;
                if (lfoPhaseRight > 1)
                {
                    lfoPhaseRight -= 1;
                }
                
                float lfoOutRight = sin(2*M_PI * lfoPhaseRight);
                
                mLFOPhase += *mRateParameter / getSampleRate();
                if ( mLFOPhase > 1 )
                {
                    mLFOPhase -= 1;
                }
                
                lfoOutLeft *= *mDepthParameter;
                lfoOutRight *= *mDepthParameter;

                float lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.005f, 0.03f);
                float lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.005f, 0.03f);

                if (*mTypeParameter == 1)
                {
                    lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.001f, 0.005f);
                    lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.001f, 0.005f);
                }
                else if (*mTypeParameter == 2)
                {
                    lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.03f, 0.1f);
                    lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.03f, 0.1f);
                }
                else if (*mTypeParameter == 3)
                {
                    lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.1f, 1.f);
                    lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.1f, 1.f);
                }
                
                float delayTimeSamplesLeft = getSampleRate() * lfoOutMappedLeft;
                float delayTimeSamplesRight = getSampleRate() * lfoOutMappedRight;
                
                
                float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSamplesLeft;
                if (delayReadHeadLeft < 0)
                {
                    delayReadHeadLeft += mCircularBufferLength;
                }
                
                float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSamplesRight;
                if (delayReadHeadRight < 0)
                {
                    delayReadHeadRight += mCircularBufferLength;
                }
                
                int readHeadLeft_x = (int)delayReadHeadLeft;
                int readHeadLeft_x1 = readHeadLeft_x + 1;
                float readHeadFloatLeft = delayReadHeadLeft - readHeadLeft_x;
                
                if ( readHeadLeft_x1 >= mCircularBufferLength )
                {
                    readHeadLeft_x1 -= mCircularBufferLength;
                }
                
                int readHeadRight_x = (int)delayReadHeadRight;
                int readHeadRight_x1 = readHeadRight_x + 1;
                float readHeadFloatRight = delayReadHeadRight - readHeadRight_x;
                
                if ( readHeadRight_x1 >= mCircularBufferLength )
                {
                    readHeadRight_x1 -= mCircularBufferLength;
                }
                
                float delaySampleLeft = linearInterpolation(mCircularBufferLeft[readHeadLeft_x], mCircularBufferLeft[readHeadLeft_x1], readHeadFloatLeft);
                float delaySampleRight = linearInterpolation(mCircularBufferRight[readHeadRight_x], mCircularBufferRight[readHeadRight_x1], readHeadFloatRight);
                
                mFeedbackLeft = delaySampleLeft * *mFeedbackParameter;
                mFeedbackRight = delaySampleRight * *mFeedbackParameter;
                
                mCircularBufferWriteHead++;
                if ( mCircularBufferWriteHead >= mCircularBufferLength )
                {
                    mCircularBufferWriteHead = 0;
                }
                
                float dryAmount = 1 - *mDryWetParameter;
                float wetAmount = *mDryWetParameter;
                
                buffer.setSample(0, i, buffer.getSample(0, i) * dryAmount + delaySampleLeft * wetAmount);
                buffer.setSample(1, i, buffer.getSample(1, i) * dryAmount + delaySampleRight * wetAmount);
            }
}

//==============================================================================
bool AntagonizerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AntagonizerAudioProcessor::createEditor()
{
    return new AntagonizerAudioProcessorEditor (*this);
}

//==============================================================================
void AntagonizerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AntagonizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AntagonizerAudioProcessor();
}

float AntagonizerAudioProcessor::linearInterpolation(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase * sample_x1;
}
