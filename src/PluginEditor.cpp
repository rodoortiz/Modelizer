#include "PluginProcessor.h"
#include "PluginEditor.h"

ModelizerAudioProcessorEditor::ModelizerAudioProcessorEditor (ModelizerAudioProcessor& p) : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible (recordButton);
    recordButton.addListener (this);
    recordButton.setColour(juce::TextButton::buttonColourId, juce::Colour (0XFFFC7558));
    recordButton.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentWhite);

    addAndMakeVisible (pauseButton);
    pauseButton.addListener (this);
    pauseButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0XFFCDCCCC));
    pauseButton.setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentWhite);

    addAndMakeVisible(processStatusLabel);
    processStatusLabel.setFont (juce::Font(20.0f));
    processStatusLabel.setColour (juce::Label::textColourId, juce::Colours::dimgrey);
    processStatusLabel.setVisible (false);
    processStatusLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(dragDropComponent);
    dragDropComponent.makeLabelVisible = [this] (juce::String inPath) {
        pathEditor = inPath;
        dragDropColor = juce::Colour (126, 127, 154);
        repaint();
    };

    setSize (500, 300);
}

ModelizerAudioProcessorEditor::~ModelizerAudioProcessorEditor()
{
    recordButton.removeListener (this);
    pauseButton.removeListener (this);
}

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

    g.setColour(dragDropColor);
    juce::Rectangle<float> borderRect { 130.0f, 230.0f };
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
    recordButton.setBoundsRelative (buttonArea);
    recordButton.setCentreRelative (0.55f, 0.27f);

    pauseButton.setBoundsRelative (buttonArea);
    pauseButton.setCentreRelative (0.77f, 0.27f);

    processStatusLabel.setBoundsRelative (0.55f, 0.62f, 0.25f, 0.1f);

    dragDropComponent.setBounds (16, 60, 130, 228);
}

void ModelizerAudioProcessorEditor::buttonClicked (juce::Button* b)
{
    if (&recordButton == b)
    {
        audioProcessor.recordingArray[0].clear();
        audioProcessor.recordingArray[1].clear();
        
        audioProcessor.isRecordingOn = true;
        
        processStatusLabel.setText ("Recording...", juce::dontSendNotification);
        processStatusLabel.setVisible(true);
    }

    else if (&pauseButton == b)
    {
        audioProcessor.isRecordingOn = false;
        
        processStatusLabel.setText ("Processing...", juce::dontSendNotification);

        processModel = std::make_unique<ThreadProcessing>(audioProcessor.recordingArray[0],
                                                          audioProcessor.recordingArray[1],
                                                          pathEditor,
                                                          processStatusLabel);
        processModel->startThread();
    }
}

// ***********************************************

DragDropComponent::DragDropComponent()
{
    addAndMakeVisible(dragDropLabel);
    dragDropLabel.setFont (juce::Font(15.0f));
    dragDropLabel.setText ("Drag and drop your model", juce::dontSendNotification);
    dragDropLabel.setColour (juce::Label::textColourId, juce::Colours::dimgrey);
    dragDropLabel.setJustificationType(juce::Justification::centred);
}

DragDropComponent::~DragDropComponent() {}

void DragDropComponent::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour (0,0,0).withAlpha(0.0f));
}

void DragDropComponent::resized()
{
    dragDropLabel.setBoundsRelative(0.5f - 0.45f, 0.5f - 0.1f, 0.9f, 0.2f);
}

bool DragDropComponent::isInterestedInFileDrag (const juce::StringArray& files)
{
    for(auto file : files)
    {
        if(file.contains(".pt"))
        {
            return true;
        }
    }

    return false;
}

void DragDropComponent::filesDropped (const juce::StringArray& files, int x, int y)
{
    for(auto file : files)
    {
        if(isInterestedInFileDrag(file))
        {
            auto myFile = std::make_unique<juce::File>(file);
            dragDropLabel.setText("Model loaded", juce::dontSendNotification);

            modelPath = myFile->getFullPathName();
            makeLabelVisible(modelPath);
        }
    }

    repaint();
}
