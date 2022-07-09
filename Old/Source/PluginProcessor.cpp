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
    formatManager.registerBasicFormats();
}

PluginAudioProcessor::~PluginAudioProcessor()
{
    formatReader = nullptr;
}

void PluginAudioProcessor::loadModelFromPytorch()
{
    try
    {
        model = torch::jit::load("/Users/rodolfoortiz/Documents/JUCE_Projects/CLion_Projects/Modelizer/ChannelKillerTS.pt");
        //model = torch::jit::load("/Users/rodolfoortiz/Documents/JUCE_Projects/CLion_Projects/Modelizer/MCClipperModelTS.pt");
        DBG("MODEL LOADED");
    }

    catch (const c10::Error& e)
    {
        DBG ("ERROR LOADING MODEL");
    }
}

void PluginAudioProcessor::processOfflineWithModel()
{
    DBG("CLICK");

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

    writer.reset(format.createWriterFor(new juce::FileOutputStream(fileOut), 44100, audioBufferOffline.getNumChannels(), 24, {}, 0));

    if (writer != nullptr)
        writer->writeFromAudioSampleBuffer(audioBufferOffline, 0, audioBufferOffline.getNumSamples());
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

    for(auto channel = 0; channel < numChannels; channel++)
    {
        auto lengthInSamples = static_cast<int>(getSampleRate() * multSamplerate);

        processedModelBuffer[channel].setSize (numChannels, lengthInSamples, false, false, true);
        //processedModelBuffer[channel].clear();

        auxBuffer[channel].setSize(numChannels, lengthInSamples, false, false, true);
        //auxBuffer[channel].clear();
    }
}

void PluginAudioProcessor::releaseResources() {}

bool PluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return true;
}

void PluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto numSamples = buffer.getNumSamples();

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

    for (auto channel = 0; channel < numChannels; channel++)
    {
        for (int i = 0; i < numSamples; i++) {
            float sample = buffer.getSample(channel, i);

            // FILL GRAIN WITH SAMPLE
            auxBuffer[channel].setSample(channel, contSampleAux[channel], sample);
            contSampleAux[channel]++;

            // GET REVERSED SAMPLE
            float outBuffer = processedModelBuffer[channel].getSample(channel, contSampleAux[channel]);

            // IF IS THE FIRST LOOP PASS NORMAL AUDIO IF NOT REVERSE SAMPLE (IN ORDER TO NOT HAVE SILENCE)
            if (contLoop[channel] > 0)
                buffer.setSample(channel, i, outBuffer);
            else
                buffer.setSample(channel, i, sample);

            // IF GRAIN IS FULL
            if (contSampleAux[channel] >= auxBuffer[channel].getNumSamples() - 1) {
                // RESET CONT LOOP
                contSampleAux[channel] = 0;

                // MAKE COPY OF LOOP AND REVERSE
                //processedModelBuffer[channel].clear();
                processedModelBuffer[channel].makeCopyOf(auxBuffer[channel], true);

                // PROCESS WITH MODEL ->>>> SE MANDA A OTRO THREAD
                processRealTimeWithModel(processedModelBuffer[channel]);

                // CLEAR LOOP
                //auxBuffer[channel].clear();

                // ADD LOOP COUNT
                contLoop[channel]++;
            }
        }
    }
}

void PluginAudioProcessor::processRealTimeWithModel(juce::AudioBuffer<float>& inBuffer)
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
