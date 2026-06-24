
namespace pedalboard {
namespace {
auto& addParameterToProcessor(juce::AudioProcessor& processor, auto parameter) {
  auto& result = *parameter;
  processor.addParameter(parameter.release());
  return result;
}

juce::AudioParameterFloat& createFloatParameter(
    juce::AudioProcessor& processor,
    const juce::String& id,
    const juce::String& name,
    float defaultValue) {
  constexpr auto versionHint = 1;
  return addParameterToProcessor(
      processor,
      std::make_unique<juce::AudioParameterFloat>(
          juce::ParameterID{id, versionHint}, name,
          juce::NormalisableRange<float>{0.f, 1.f, 0.001f}, defaultValue,
          juce::AudioParameterFloatAttributes{}.withLabel("%")));
}

juce::AudioParameterBool& createBypassedParameter(
    juce::AudioProcessor& processor) {
  constexpr auto versionHint = 1;
  return addParameterToProcessor(
      processor,
      std::make_unique<juce::AudioParameterBool>(
          juce::ParameterID{"bypassed", versionHint}, "Bypass", false));
}

}  // namespace

Parameters::Parameters(juce::AudioProcessor& processor)
    : bypassed{createBypassedParameter(processor)},
      reverbRoomSize{createFloatParameter(
          processor, "reverb.roomSize", "Reverb room size", 0.55f)},
      reverbDamping{createFloatParameter(
          processor, "reverb.damping", "Reverb damping", 0.45f)},
      reverbMix{
          createFloatParameter(processor, "reverb.mix", "Reverb mix", 0.3f)},
      reverbWidth{createFloatParameter(
          processor, "reverb.width", "Reverb width", 1.f)} {}
}  // namespace pedalboard
