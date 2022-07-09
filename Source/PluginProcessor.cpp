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
    loadModelFromPytorch();
}

PluginAudioProcessor::~PluginAudioProcessor()
{

}

void PluginAudioProcessor::loadModelFromPytorch()
{
    try
    {
        model = torch::jit::load("/Users/rodolfoortiz/Documents/JUCE_Projects/CLion_Projects/Modelizer/ChannelKillerTS.pt");
        DBG("MODEL LOADED");
    }

    catch (const c10::Error& e)
    {
        DBG ("ERROR LOADING MODEL");
    }
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
    sizeBuffer = getBlockSize();
    numChannels = getTotalNumInputChannels();
}

void PluginAudioProcessor::releaseResources() {}

bool PluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}

void PluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    modelBuffer.makeCopyOf(buffer);

    std::vector<int64_t> blockSize = { buffer.getNumSamples() };

    auto* modelBufferDataL = modelBuffer.getWritePointer(0);
    auto* modelBufferDataR = modelBuffer.getWritePointer(1);

    at::Tensor tensorFrameL = torch::from_blob(modelBufferDataL, blockSize);
    at::Tensor tensorFrameR = torch::from_blob(modelBufferDataR, blockSize);
    at::Tensor tensorFrame = at::stack({tensorFrameL, tensorFrameR});

    tensorFrame = torch::reshape(tensorFrame, { 1, numChannels, sizeBuffer });

    std::vector<torch::jit::IValue> inputs;
    inputs.emplace_back(tensorFrame);

    auto outputFrame = model.forward(inputs).toTensor();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto outputData = outputFrame.index({0, channel, torch::indexing::Slice()});
        auto outputDataPtr = outputData.data_ptr<float>();
        buffer.copyFrom (channel, 0, outputDataPtr, sizeBuffer);
    }
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
