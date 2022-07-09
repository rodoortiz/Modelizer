#include <JuceHeader.h>
#include "PluginProcessor.h"

class PluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    PluginAudioProcessorEditor(PluginAudioProcessor &p);

    ~PluginAudioProcessorEditor() override;

    void paint(juce::Graphics &g) override;

    void resized() override;

private:

    PluginAudioProcessor& audioProcessor;

};
