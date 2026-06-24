#pragma once

namespace pedalboard {
struct ReverbSettings {
  float roomSize = 0.55f;
  float damping = 0.45f;
  float mix = 0.3f;
  float width = 1.f;
};

class ReverbProcessor {
public:
  void prepare(const juce::dsp::ProcessSpec& spec) {
    reverb.prepare(spec);
    reset();
  }

  void reset() noexcept {
    reverb.reset();
  }

  void process(juce::AudioBuffer<float>& buffer,
               const ReverbSettings& settings) noexcept {
    juce::dsp::Reverb::Parameters parameters;
    parameters.roomSize = settings.roomSize;
    parameters.damping = settings.damping;
    parameters.wetLevel = settings.mix;
    parameters.dryLevel = 1.f - settings.mix;
    parameters.width = settings.width;
    parameters.freezeMode = 0.f;
    reverb.setParameters(parameters);

    auto block = juce::dsp::AudioBlock<float>{buffer};
    auto context = juce::dsp::ProcessContextReplacing<float>{block};
    reverb.process(context);
  }

private:
  juce::dsp::Reverb reverb;
};
}  // namespace pedalboard
