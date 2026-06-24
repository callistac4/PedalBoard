#pragma once

namespace pedalboard {
class JsonSerializer {
public:
  static void serialize(
      const Parameters&,
      const std::array<PedalType, pedalSlotCount>&,
      juce::OutputStream&);

  /** @return Error message on failure; empty string otherwise.
   *           In case of error, no parameters are updated. */
  static juce::Result deserialize(
      juce::InputStream&,
      Parameters&,
      std::array<PedalType, pedalSlotCount>&);
};
}  // namespace pedalboard
