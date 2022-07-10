#include "PluginProcessor.h"
#include "PluginEditor.h"

ModelizerAudioProcessorEditor::ModelizerAudioProcessorEditor (ModelizerAudioProcessor& p) : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible (button);
    button.addListener (this);

    setSize (600, 300);
}

ModelizerAudioProcessorEditor::~ModelizerAudioProcessorEditor()
{
}

void ModelizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
}

void ModelizerAudioProcessorEditor::resized()
{
    juce::Rectangle<float> buttonArea { 0.2f, 0.2f };
    button.setBoundsRelative (buttonArea);
    button.setCentreRelative (0.5f, 0.5f);
}

void ModelizerAudioProcessorEditor::buttonClicked (juce::Button* b)
{
    if (&button == b)
    {
        processModel = std::make_unique<ThreadProcessing>();
        processModel->startThread();
    }
}

