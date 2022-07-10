#include "PluginProcessor.h"
#include "PluginEditor.h"

ModelizerAudioProcessorEditor::ModelizerAudioProcessorEditor (ModelizerAudioProcessor& p) : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (500, 300);
}

ModelizerAudioProcessorEditor::~ModelizerAudioProcessorEditor() {}

void ModelizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);

    juce::Rectangle<int> headerRect {getWidth(), getHeight() / 6};
    g.setColour (juce::Colour (0XFFFC7558));
    g.fillRect (headerRect);

    juce::Rectangle<int> textRect { 200, 20 };
    textRect.setCentre(250, 24);
    g.setFont (24.0f);
    g.setColour (juce::Colours::white);
    g.drawText ("MODULIZER", textRect, juce::Justification::centred);

    g.setColour(juce::Colour (0XFFDFDFDF));
    juce::Rectangle<float> borderRect {130.0f, 230.0f};
    borderRect.setCentre(80,175);
    g.drawRoundedRectangle(borderRect, 10.0f, 2.0f);

    g.setColour(juce::Colour (0XFFDFDFDF));
    juce::Rectangle<float> borderRect2 { 310.0f, 180.0f };
    borderRect2.setCentre(325,200);
    g.drawRoundedRectangle(borderRect2, 10.0f, 2.0f);
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
        audioProcessor.processOfflineWithModel();
}

