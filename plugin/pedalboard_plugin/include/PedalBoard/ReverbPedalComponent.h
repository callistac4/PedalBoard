#pragma once

namespace pedalboard {
class ReverbPedalComponent : public juce::Component {
public:
  using DeleteCallback = std::function<void()>;
  using MoveCallback = std::function<void(size_t)>;

  ReverbPedalComponent(Parameters&,
                       std::optional<size_t> slot,
                       DeleteCallback,
                       MoveCallback);
  ~ReverbPedalComponent() override;

  void paint(juce::Graphics&) override;
  void resized() override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;

private:
  void configureKnob(juce::Slider&);
  void showContextMenu();

  std::optional<size_t> slotIndex;
  DeleteCallback onDelete;
  MoveCallback onMove;
  bool dragStarted = false;

  juce::Slider mixKnob;
  juce::Slider roomKnob;
  juce::Slider dampingKnob;
  juce::SliderParameterAttachment mixAttachment;
  juce::SliderParameterAttachment roomAttachment;
  juce::SliderParameterAttachment dampingAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbPedalComponent)
};
}  // namespace pedalboard
