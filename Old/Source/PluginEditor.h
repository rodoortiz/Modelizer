#include <JuceHeader.h>
#include "PluginProcessor.h"

class PluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   public juce::Button::Listener
{
public:
    PluginAudioProcessorEditor(PluginAudioProcessor &p);

    ~PluginAudioProcessorEditor() override;

    void paint(juce::Graphics &g) override;

    void resized() override;

    void buttonClicked (juce::Button* b) override;

private:
    PluginAudioProcessor& audioProcessor;

    juce::TextButton button { "CLICK" };

};
