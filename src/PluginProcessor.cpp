#include "PluginProcessor.h"
#include "PluginEditor.h"

ModelizerAudioProcessor::ModelizerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{

}

ModelizerAudioProcessor::~ModelizerAudioProcessor()
{

}

const String ModelizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ModelizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ModelizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ModelizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ModelizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ModelizerAudioProcessor::getNumPrograms()
{
    return 1;
} 

int ModelizerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ModelizerAudioProcessor::setCurrentProgram ([[maybe_unused]] int index){}

const String ModelizerAudioProcessor::getProgramName ([[maybe_unused]] int index)
{
    return {};
}

void ModelizerAudioProcessor::changeProgramName ([[maybe_unused]] int index, [[maybe_unused]] const String& newName){}

void ModelizerAudioProcessor::prepareToPlay ([[maybe_unused]] double sampleRate_, int samplesPerBlock_)
{

}

void ModelizerAudioProcessor::releaseResources(){}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ModelizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif
    return true;
  #endif
}
#endif

void ModelizerAudioProcessor::processBlock (AudioBuffer<float>& buffer, [[maybe_unused]] MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;



    // Process sequentially in real time with Model with many buffer
    /*for (auto channel = 0; channel < numChannels; channel++)
    {
        for (int i = 0; i < numSamples; i++) {
            float sample = buffer.getSample(channel, i);

            // FILL LOOP WITH SAMPLE
            auxBuffer[channel].setSample(channel, contSampleAux[channel], sample);
            contSampleAux[channel]++;

            // GET LOOP SAMPLE
            float outBuffer = processedModelBuffer[channel].getSample(channel, contSampleAux[channel]);

            // IF IS THE FIRST LOOP PASS NORMAL AUDIO IF NOT LOOP SAMPLE (IN ORDER TO NOT HAVE SILENCE)
            if (contLoop[channel] > 0)
                buffer.setSample(channel, i, outBuffer);
            else
                buffer.setSample(channel, i, sample);

            // IF LOOP IS FULL
            if (contSampleAux[channel] >= auxBuffer[channel].getNumSamples() - 1) {
                // RESET CONT LOOP
                contSampleAux[channel] = 0;

                // MAKE COPY OF LOOP
                //processedModelBuffer[channel].clear();
                processedModelBuffer[channel].makeCopyOf(auxBuffer[channel], true);

                // PROCESS WITH MODEL
                processRealTimeWithModel(processedModelBuffer[channel]);

                // CLEAR LOOP
                //auxBuffer[channel].clear();

                // ADD LOOP COUNT
                contLoop[channel]++;
            }
        }
    }*/
}

bool ModelizerAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* ModelizerAudioProcessor::createEditor()
{
    return new ModelizerAudioProcessorEditor (*this);
}

void ModelizerAudioProcessor::getStateInformation ([[maybe_unused]] MemoryBlock& destData)
{

}

void ModelizerAudioProcessor::setStateInformation ([[maybe_unused]] const void* data, [[maybe_unused]] int sizeInBytes)
{

}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ModelizerAudioProcessor();
}
