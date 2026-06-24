#pragma once

namespace pedalboard {
class PedalBoard {
public:
  void prepare(double sampleRate,
               int expectedMaxFramesPerBlock,
               int channelCount) {
    const juce::dsp::ProcessSpec processSpec{
        .sampleRate = sampleRate,
        .maximumBlockSize =
            static_cast<juce::uint32>(expectedMaxFramesPerBlock),
        .numChannels = static_cast<juce::uint32>(channelCount),
    };
    for (auto& processor : reverbProcessors) {
      processor.prepare(processSpec);
    }
  }

  void process(juce::AudioBuffer<float>& buffer,
               const ReverbSettings& reverbSettings) noexcept {
    for (auto slot = 0u; slot < pedalSlotCount; ++slot) {
      switch (getPedal(slot)) {
        case PedalType::reverb:
          reverbProcessors[slot].process(buffer, reverbSettings);
          break;
        case PedalType::empty:
          break;
      }
    }
  }

  void reset() noexcept {
    for (auto& processor : reverbProcessors) {
      processor.reset();
    }
  }

  [[nodiscard]] PedalType getPedal(size_t slot) const noexcept {
    jassert(slot < pedalSlotCount);
    if (slot >= pedalSlotCount) {
      return PedalType::empty;
    }
    return slots[slot].load(std::memory_order_relaxed);
  }

  [[nodiscard]] std::array<PedalType, pedalSlotCount> getPedals()
      const noexcept {
    std::array<PedalType, pedalSlotCount> result{};
    for (auto slot = 0u; slot < pedalSlotCount; ++slot) {
      result[slot] = getPedal(slot);
    }
    return result;
  }

  void setPedal(size_t slot, PedalType type) noexcept {
    jassert(slot < pedalSlotCount);
    if (slot < pedalSlotCount) {
      slots[slot].store(type, std::memory_order_relaxed);
    }
  }

  void setPedals(
      const std::array<PedalType, pedalSlotCount>& newSlots) noexcept {
    for (auto slot = 0u; slot < pedalSlotCount; ++slot) {
      setPedal(slot, newSlots[slot]);
    }
  }

  bool movePedal(size_t source, size_t destination) noexcept {
    if (source >= pedalSlotCount || destination >= pedalSlotCount ||
        source == destination || getPedal(source) == PedalType::empty ||
        getPedal(destination) != PedalType::empty) {
      return false;
    }

    const auto type = getPedal(source);
    setPedal(source, PedalType::empty);
    setPedal(destination, type);
    return true;
  }

private:
  std::array<std::atomic<PedalType>, pedalSlotCount> slots{};
  std::array<ReverbProcessor, pedalSlotCount> reverbProcessors;
};
}  // namespace pedalboard
