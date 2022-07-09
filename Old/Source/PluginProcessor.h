#pragma once
#include <JuceHeader.h>
#include <torch/script.h>

class PluginAudioProcessor  : public juce::AudioProcessor
{
public:

    PluginAudioProcessor();
    ~PluginAudioProcessor() override;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void loadModelFromPytorch();

    void processOfflineWithModel();

    void processRealTimeWithModel(juce::AudioBuffer<float>& inBuffer);

private:

    // General data
    int sizeBuffer;
    int numChannels;

    // Model data
    torch::jit::script::Module model;
    juce::AudioSampleBuffer modelBuffer;

    // Offline data
    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader* formatReader {nullptr};

    juce::AudioBuffer<float> processedModelBuffer[2];
    juce::AudioBuffer<float> auxBuffer[2];

    int contSampleAux[2];
    int contLoop[2];
    float multSamplerate {0.125f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessor)
};
