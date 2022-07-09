#pragma once
#include <JuceHeader.h>
#include <torch/script.h>

class ModelizerAudioProcessor : public juce::AudioProcessor
{
public:

    ModelizerAudioProcessor();
    ~ModelizerAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    #ifndef JucePlugin_PreferredChannelConfigurations
        bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    #endif
    void processBlock (juce::AudioBuffer<float>&, MidiBuffer&) override;
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void loadModelFromPytorch();

    void processOfflineWithModel();

    void processRealTimeWithModel (juce::AudioBuffer<float>& inBuffer);

private:

    // General data
    int sizeBuffer {0};
    int numChannels {0};

    // Model data
    torch::jit::script::Module model;
    juce::AudioSampleBuffer modelBuffer;

    // Offline data
    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader* formatReader {nullptr};

    // Buffers Processing data
    juce::AudioBuffer<float> processedModelBuffer[2];
    juce::AudioBuffer<float> auxBuffer[2];

    int contSampleAux[2] {0};
    int contLoop[2] {0};
    float multSamplerate {0.125f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModelizerAudioProcessor)

};
