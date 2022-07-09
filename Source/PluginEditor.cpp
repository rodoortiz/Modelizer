#include "PluginEditor.h"

PluginAudioProcessorEditor::PluginAudioProcessorEditor(PluginAudioProcessor &p) : AudioProcessorEditor(p), audioProcessor(p)
{
    setSize(500, 500);
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor(){}

void PluginAudioProcessorEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colours::dimgrey);
}

void PluginAudioProcessorEditor::resized()
{
    
}