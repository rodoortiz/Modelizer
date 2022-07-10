#include "PluginProcessor.h"
#include "PluginEditor.h"

ModelizerAudioProcessorEditor::ModelizerAudioProcessorEditor (ModelizerAudioProcessor& p) : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible (playButton);
    playButton.addListener (this);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colour (0XFFFC7558));
    playButton.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentWhite);

    addAndMakeVisible (pauseButton);
    pauseButton.addListener (this);
    pauseButton.setColour(juce::TextButton::buttonColourId, juce::Colour (0XFFCDCCCC));
    pauseButton.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentWhite);

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
    juce::Rectangle<float> buttonArea { 0.2f, 0.1f };
    playButton.setBoundsRelative (buttonArea);
    playButton.setCentreRelative (0.55f, 0.27f);

    pauseButton.setBoundsRelative (buttonArea);
    pauseButton.setCentreRelative (0.77f, 0.27f);
}

void ModelizerAudioProcessorEditor::buttonClicked (juce::Button* b)
{
    if (&playButton == b)
    {
        
    }
    else if (&pauseButton == b)
    {
        processModel = std::make_unique<ThreadProcessing>();
        processModel->startThread();
    }
}

