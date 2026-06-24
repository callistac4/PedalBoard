#pragma once

namespace pedalboard {
struct Parameters {
  explicit Parameters(juce::AudioProcessor&);

  juce::AudioParameterBool& bypassed;
  juce::AudioParameterFloat& reverbRoomSize;
  juce::AudioParameterFloat& reverbDamping;
  juce::AudioParameterFloat& reverbMix;
  juce::AudioParameterFloat& reverbWidth;

  JUCE_DECLARE_NON_COPYABLE(Parameters)
  JUCE_DECLARE_NON_MOVEABLE(Parameters)
};
}  // namespace pedalboard
