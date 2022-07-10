#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Processing in thread
class ThreadProcessing : public juce::Thread
{
public:
    
    ThreadProcessing (std::vector<float>& array, std::vector<float>& arrayTwo, juce::String inPath, juce::Label& inLabel) : juce::Thread("Buffer Thread"), vectorDataOne(array), vectorDataTwo(arrayTwo), path(inPath.toStdString()), label(inLabel)
    {
        formatManager.registerBasicFormats();

        recordedBuffer.setSize(2, (int)array.size());

        for (int channel = 0; channel < 2; ++channel)
        {
            for (size_t i = 0; i < recordedBuffer.getNumSamples(); ++i)
            {
                float sample;

                if (channel == 0)
                    sample = vectorDataOne[i];
                else
                    sample = vectorDataTwo[i];

                recordedBuffer.setSample(channel, i, sample);
            }
        }
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
        try
        {            
            DBG(path);
            model = std::make_unique<torch::jit::script::Module>(torch::jit::load(path));
            DBG("MODEL LOADED");
        }
        
        catch (const c10::Error& e)
        {
            DBG ("ERROR LOADING MODEL");
            DBG(e.msg());
        }
        
        // *************************** Import audio ********************* //
        juce::AudioBuffer<float> audioBufferOffline;

        audioBufferOffline.makeCopyOf (recordedBuffer);

        // ************************ Process with model ***************** //
        juce::AudioBuffer<float> bufferOfflineProcessed;
        bufferOfflineProcessed.makeCopyOf(recordedBuffer);

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
        juce::File fileOut( juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getFullPathName() + "/OutSignalModel.wav");
        
        fileOut.deleteFile();

        juce::WavAudioFormat format;
        std::unique_ptr<juce::AudioFormatWriter> writer;

        writer.reset(format.createWriterFor(new juce::FileOutputStream(fileOut),
                                            48000,
                                            static_cast<unsigned int>(audioBufferOffline.getNumChannels()),
                                            24,
                                            {},
                                            0));

        if (writer != nullptr)
            writer->writeFromAudioSampleBuffer(audioBufferOffline, 0, audioBufferOffline.getNumSamples());
        
        juce::MessageManager::callAsync([&]
        {
            label.setText ("Ready!", juce::dontSendNotification);
        });
        
        DBG("Finished thread processing");
    }
    
private:
    
    // General data
    int sizeBuffer {0};
    int numChannels {0};
    
    // Offline data
    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader* formatReader {nullptr};
    
    // Model
    std::unique_ptr<torch::jit::script::Module> model;

    std::vector<float>& vectorDataOne;
    std::vector<float>& vectorDataTwo;

    juce::AudioBuffer<float> recordedBuffer;
    
    std::string path{""};
    
    juce::Label& label;
    
    
    
};

class DragDropComponent : public juce::Component, public juce::FileDragAndDropTarget
{
public:
    
    DragDropComponent();
    ~DragDropComponent() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    
    std::function<void(juce::String inPath)> makeLabelVisible;
    
private:
    
    juce::String modelPath {""};
    juce::Label dragDropLabel;
    
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
    
    juce::String pathEditor {""};

    juce::Label processStatusLabel;

    std::unique_ptr<ThreadProcessing> processModel;
    
    DragDropComponent dragDropComponent;
    juce::Colour dragDropColor {223,223,233};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModelizerAudioProcessorEditor)
};
