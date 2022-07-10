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
    loadModelFromPytorch();
    formatManager.registerBasicFormats();
}

ModelizerAudioProcessor::~ModelizerAudioProcessor()
{
    formatReader = nullptr;
}

void ModelizerAudioProcessor::loadModelFromPytorch()
{
    try
    {
        model = torch::jit::load("/Users/rodolfoortiz/Documents/JUCE_Projects/CLion_Projects/Modelizer/ClipperTSr.pt");

        DBG("MODEL LOADED");
    }

    catch (const c10::Error& e)
    {
        DBG ("ERROR LOADING MODEL");
    }
}

void ModelizerAudioProcessor::processOfflineWithModel()
{
    DBG("Starting offline...");

    // *********************** Import audio ******************** //
    juce::AudioBuffer<float> audioBufferOffline;

    auto fileLoaded = juce::File("/Users/rodolfoortiz/Downloads/TestZafiro Guitar 1.wav");
    formatReader = formatManager.createReaderFor(fileLoaded);

    auto sampleLength = static_cast<int>(formatReader->lengthInSamples);
    audioBufferOffline.setSize(2, sampleLength);

    formatReader->read(&audioBufferOffline, 0, sampleLength, 0, true, false);

    // ****************** Process with model ************************ //
    juce::AudioBuffer<float> bufferOfflineProcessed;
    bufferOfflineProcessed.makeCopyOf(audioBufferOffline);

    sizeBuffer = sampleLength;
    std::vector<int64_t> blockSize = { sizeBuffer };

    auto* modelBufferDataL = bufferOfflineProcessed.getWritePointer(0);
    auto* modelBufferDataR = bufferOfflineProcessed.getWritePointer(1);

    at::Tensor tensorFrameL = torch::from_blob(modelBufferDataL, blockSize);
    at::Tensor tensorFrameR = torch::from_blob(modelBufferDataR, blockSize);
    at::Tensor tensorFrame = at::stack({tensorFrameL, tensorFrameR});

    tensorFrame = torch::reshape(tensorFrame, { 1, numChannels, sizeBuffer });

    std::vector<torch::jit::IValue> inputs;
    inputs.emplace_back(tensorFrame);

    auto outputFrame = model.forward(inputs).toTensor();

    for (int channel = 0; channel < 2; ++channel)
    {
        auto outputData = outputFrame.index({0, channel, torch::indexing::Slice()});
        auto outputDataPtr = outputData.data_ptr<float>();
        audioBufferOffline.copyFrom (channel, 0, outputDataPtr, sizeBuffer);
    }

    // *********************** Export audio ******************** //
    juce::File fileOut("/Users/rodolfoortiz/Downloads/test.wav");
    fileOut.deleteFile();

    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatWriter> writer;

    writer.reset(format.createWriterFor(new juce::FileOutputStream(fileOut),
                                        44100,
                                        static_cast<unsigned int>(audioBufferOffline.getNumChannels()),
                                        24,
                                        {},
                                        0));

    if (writer != nullptr)
        writer->writeFromAudioSampleBuffer(audioBufferOffline, 0, audioBufferOffline.getNumSamples());
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
    sizeBuffer = samplesPerBlock_;
    numChannels = getTotalNumInputChannels();

    for(auto channel = 0; channel < numChannels; channel++)
    {
        auto lengthInSamples = static_cast<int>(getSampleRate() * multSamplerate);

        processedModelBuffer[channel].setSize (numChannels, lengthInSamples, false, false, true);
        //processedModelBuffer[channel].clear();

        auxBuffer[channel].setSize(numChannels, lengthInSamples, false, false, true);
        //auxBuffer[channel].clear();
    }
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

    auto numSamples = buffer.getNumSamples();

    // Process sequentially in real time with Model with one buffer
    /*modelBuffer.makeCopyOf(buffer);

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
    }*/

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

void ModelizerAudioProcessor::processRealTimeWithModel (juce::AudioBuffer<float>& inBuffer)
{
    modelBuffer.makeCopyOf(inBuffer);

    std::vector<int64_t> blockSize = { inBuffer.getNumSamples() };

    auto* modelBufferDataL = modelBuffer.getWritePointer(0);
    auto* modelBufferDataR = modelBuffer.getWritePointer(1);

    at::Tensor tensorFrameL = torch::from_blob(modelBufferDataL, blockSize);
    at::Tensor tensorFrameR = torch::from_blob(modelBufferDataR, blockSize);
    at::Tensor tensorFrame = at::stack({tensorFrameL, tensorFrameR});

    sizeBuffer = static_cast<int>(getSampleRate() * multSamplerate);
    tensorFrame = torch::reshape(tensorFrame, { 1, numChannels, sizeBuffer });

    std::vector<torch::jit::IValue> inputs;
    inputs.emplace_back(tensorFrame);

    auto outputFrame = model.forward(inputs).toTensor();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto outputData = outputFrame.index({0, channel, torch::indexing::Slice()});
        auto outputDataPtr = outputData.data_ptr<float>();
        inBuffer.copyFrom (channel, 0, outputDataPtr, sizeBuffer);
    }
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
