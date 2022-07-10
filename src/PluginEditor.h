#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Processing in thread
class ThreadProcessing : public juce::Thread
{
public:
    
    ThreadProcessing() : juce::Thread("Buffer Thread")
    {
        formatManager.registerBasicFormats();
    }
    
    ~ThreadProcessing() override
    {
        formatReader = nullptr;
    }
    
    void run() override
    {
        processBuffer();
    }
    
    void processBuffer()
    {
        DBG("Starting processing in thread...");
        
        //**************************** Load Model *********************** //
        std::unique_ptr<torch::jit::script::Module> model;
        //model = std::make_unique<torch::jit::script::Module>(torch::jit::load("/Users/rodolfoortiz/Documents/JUCE_Projects/CLion_Projects/Modelizer/ClipperTSr.pt"));
        
        model = std::make_unique<torch::jit::script::Module>(torch::jit::load("Users/jsvaldezv/Documents/GitHub/Personal/Modelizer/ClipperTSFinal.pt"));
        
        // *************************** Import audio ********************* //
        juce::AudioBuffer<float> audioBufferOffline;

        //auto fileLoaded = juce::File("/Users/rodolfoortiz/Downloads/TestZafiro Guitar 1.wav");
        auto fileLoaded = juce::File("/Users/jsvaldezv/Documents/GitHub/Personal/Modelizer/TestOriginal.wav");
        formatReader = formatManager.createReaderFor(fileLoaded);

        auto sampleLength = static_cast<int>(formatReader->lengthInSamples);
        audioBufferOffline.setSize(2, sampleLength);

        formatReader->read(&audioBufferOffline, 0, sampleLength, 0, true, false);

        // ************************ Process with model ***************** //
        juce::AudioBuffer<float> bufferOfflineProcessed;
        bufferOfflineProcessed.makeCopyOf(audioBufferOffline);

        sizeBuffer = bufferOfflineProcessed.getNumSamples();
        numChannels = bufferOfflineProcessed.getNumChannels();
        
        std::vector<int64_t> blockSize = { sizeBuffer };

        auto* modelBufferDataL = bufferOfflineProcessed.getWritePointer(0);
        auto* modelBufferDataR = bufferOfflineProcessed.getWritePointer(1);

        at::Tensor tensorFrameL = torch::from_blob(modelBufferDataL, blockSize);
        at::Tensor tensorFrameR = torch::from_blob(modelBufferDataR, blockSize);
        at::Tensor tensorFrame = at::stack({tensorFrameL, tensorFrameR});

        tensorFrame = torch::reshape(tensorFrame, { 1, numChannels, sizeBuffer });

        std::vector<torch::jit::IValue> inputs;
        inputs.emplace_back(tensorFrame);

        auto outputFrame = model->forward(inputs).toTensor();

        for (int channel = 0; channel < 2; ++channel)
        {
            auto outputData = outputFrame.index({0, channel, torch::indexing::Slice()});
            auto outputDataPtr = outputData.data_ptr<float>();
            audioBufferOffline.copyFrom (channel, 0, outputDataPtr, sizeBuffer);
        }

        // *********************** Export audio ******************** //
        //juce::File fileOut("/Users/rodolfoortiz/Downloads/test.wav");
        juce::File fileOut("/Users/jsvaldezv/Downloads/test.wav");
        fileOut.deleteFile();

        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatWriter> writer;

        writer.reset(format.createWriterFor(new juce::FileOutputStream(fileOut),
                                            formatReader->sampleRate,
                                            static_cast<unsigned int>(audioBufferOffline.getNumChannels()),
                                            static_cast<unsigned int>(formatReader->bitsPerSample),
                                            {},
                                            0));

        if (writer != nullptr)
            writer->writeFromAudioSampleBuffer(audioBufferOffline, 0, audioBufferOffline.getNumSamples());
        
        DBG("Finished thread processing");
    }
    
private:
    
    // General data
    int sizeBuffer {0};
    int numChannels {0};
    
    // Offline data
    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader* formatReader {nullptr};
    
};

class ModelizerAudioProcessorEditor  : public AudioProcessorEditor,
                                       public juce::Button::Listener
{
public:

    ModelizerAudioProcessorEditor (ModelizerAudioProcessor&);
    ~ModelizerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked (juce::Button* b) override;

private:

    ModelizerAudioProcessor& audioProcessor;

    juce::TextButton playButton { "Play" };
    juce::TextButton pauseButton { "Pause" };

    juce::Label processStatusLabel;

    std::unique_ptr<ThreadProcessing> processModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModelizerAudioProcessorEditor)
};
