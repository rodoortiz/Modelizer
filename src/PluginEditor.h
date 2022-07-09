#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

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

    juce::TextButton button { "CLICK" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModelizerAudioProcessorEditor)
};
