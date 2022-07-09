#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginAudioProcessor::PluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{

}

PluginAudioProcessor::~PluginAudioProcessor()
{

}

const juce::String PluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int PluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginAudioProcessor::setCurrentProgram (int /*index*/){}

const juce::String PluginAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void PluginAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/) {}

void PluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

}

void PluginAudioProcessor::releaseResources() {}

bool PluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}

void PluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

}

bool PluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PluginAudioProcessor::createEditor()
{
    return new PluginAudioProcessorEditor(*this);
}

void PluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData){}

void PluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes){}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginAudioProcessor();
}
