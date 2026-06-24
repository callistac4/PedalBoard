#pragma once

namespace pedalboard {
constexpr auto pedalSlotCount = 10u;

enum class PedalType : int {
  empty = 0,
  reverb = 1,
};

inline juce::String pedalTypeToString(PedalType type) {
  switch (type) {
    case PedalType::reverb:
      return "Reverb";
    case PedalType::empty:
      return "Empty";
  }

  jassertfalse;
  return "Empty";
}

inline std::optional<PedalType> pedalTypeFromString(const juce::String& name) {
  if (name == "Empty") {
    return PedalType::empty;
  }
  if (name == "Reverb") {
    return PedalType::reverb;
  }
  return std::nullopt;
}
}  // namespace pedalboard
