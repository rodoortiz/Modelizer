#include "PluginEditor.h"

PluginAudioProcessorEditor::PluginAudioProcessorEditor(PluginAudioProcessor &p) : AudioProcessorEditor(p), audioProcessor(p)
{
    addAndMakeVisible (button);
    button.addListener (this);

    setSize(500, 500);
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor(){}

void PluginAudioProcessorEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::dimgrey);
}

void PluginAudioProcessorEditor::resized()
{
    juce::Rectangle<float> buttonArea { 0.2f, 0.2f };
    button.setBoundsRelative (buttonArea);
    button.setCentreRelative (0.5f, 0.5f);
}

void PluginAudioProcessorEditor::buttonClicked (juce::Button* b)
{
    if (&button == b)
    {
        audioProcessor.processOfflineWithModel();
    }
}
