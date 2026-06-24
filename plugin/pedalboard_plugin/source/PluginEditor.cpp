namespace pedalboard {
namespace {
constexpr auto deleteMenuId = 1;
constexpr auto moveMenuBaseId = 100;

class PedalSlotComponent : public juce::Component,
                           public juce::DragAndDropTarget {
public:
  using DropCallback = std::function<void(size_t, const juce::var&)>;

  PedalSlotComponent(size_t slot, DropCallback callback)
      : slotIndex{slot}, onDrop{std::move(callback)} {}

  bool isInterestedInDragSource(
      const SourceDetails& details) override {
    return details.description.isString();
  }

  void itemDragEnter(const SourceDetails&) override {
    dragOver = true;
    repaint();
  }

  void itemDragExit(const SourceDetails&) override {
    dragOver = false;
    repaint();
  }

  void itemDropped(const SourceDetails& details) override {
    dragOver = false;
    onDrop(slotIndex, details.description);
    repaint();
  }

  void paint(juce::Graphics& g) override {
    const auto bounds = getLocalBounds().toFloat().reduced(3.f);
    const auto fill = dragOver ? juce::Colour{0xff5f6570}
                               : juce::Colour{0xff33373d};

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 5.f);
    g.setColour(dragOver ? juce::Colour{0xff9b5cff}
                         : juce::Colour{0xff4b5058});
    g.drawRoundedRectangle(bounds, 5.f, dragOver ? 3.f : 2.f);

    if (getNumChildComponents() == 0) {
      g.setColour(juce::Colour{0xff8c929b});
      g.setFont(juce::FontOptions{13.f});
      g.drawText("SLOT " + juce::String{static_cast<int>(slotIndex + 1)},
                 getLocalBounds(), juce::Justification::centred);
    }
  }

  void resized() override {
    if (auto* child = getChildComponent(0)) {
      child->setBounds(getLocalBounds().reduced(7));
    }
  }

private:
  size_t slotIndex;
  DropCallback onDrop;
  bool dragOver = false;
};

juce::String dragDescription(std::optional<size_t> slot) {
  return slot.has_value() ? "slot:" + juce::String{static_cast<int>(*slot)}
                          : "new:reverb";
}

std::optional<size_t> sourceSlotFromDescription(
    const juce::String& description) {
  if (!description.startsWith("slot:")) {
    return std::nullopt;
  }
  const auto slot = description.fromFirstOccurrenceOf(":", false, false)
                        .getIntValue();
  if (slot < 0 || slot >= static_cast<int>(pedalSlotCount)) {
    return std::nullopt;
  }
  return static_cast<size_t>(slot);
}
}  // namespace

ReverbPedalComponent::ReverbPedalComponent(
    Parameters& parameters,
    std::optional<size_t> slot,
    DeleteCallback deleteCallback,
    MoveCallback moveCallback)
    : slotIndex{slot},
      onDelete{std::move(deleteCallback)},
      onMove{std::move(moveCallback)},
      mixAttachment{parameters.reverbMix, mixKnob},
      roomAttachment{parameters.reverbRoomSize, roomKnob},
      dampingAttachment{parameters.reverbDamping, dampingKnob} {
  setName("Reverb");
  setRepaintsOnMouseActivity(true);
  configureKnob(mixKnob);
  configureKnob(roomKnob);
  configureKnob(dampingKnob);
}

ReverbPedalComponent::~ReverbPedalComponent() = default;

void ReverbPedalComponent::configureKnob(juce::Slider& knob) {
  knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  knob.setPopupDisplayEnabled(true, false, this);
  addAndMakeVisible(knob);
}

void ReverbPedalComponent::paint(juce::Graphics& g) {
  const auto bounds = getLocalBounds().toFloat();
  const auto body = bounds.reduced(2.f);

  auto bodyGradient = juce::ColourGradient::vertical(
      juce::Colour{0xff743bb2}, juce::Colour{0xff331052}, body);
  bodyGradient.addColour(0.6, juce::Colour{0xff4d176f});
  g.setGradientFill(bodyGradient);
  g.fillRoundedRectangle(body, 6.f);

  g.setColour(juce::Colour{0xff1d2025});
  g.drawRoundedRectangle(body, 6.f, 3.f);

  g.setColour(juce::Colours::white.withAlpha(0.9f));
  g.setFont(juce::FontOptions{juce::jlimit(11.f, 18.f, bounds.getWidth() * 0.11f)}
                .withStyle("Bold"));
  g.drawText("REVERB", getLocalBounds().removeFromBottom(
                           juce::roundToInt(bounds.getHeight() * 0.22f)),
             juce::Justification::centred);

  const auto ledBounds =
      juce::Rectangle<float>{0.f, 0.f, 9.f, 9.f}.withCentre(
          {bounds.getCentreX(), bounds.getHeight() * 0.66f});
  g.setColour(juce::Colour{0xffd5ff68});
  g.fillEllipse(ledBounds);
  g.setColour(juce::Colours::black.withAlpha(0.45f));
  g.drawEllipse(ledBounds, 1.f);
}

void ReverbPedalComponent::resized() {
  const auto size =
      juce::jlimit(26, 44,
                   juce::roundToInt(static_cast<float>(getWidth()) * 0.29f));
  mixKnob.setBounds((getWidth() - size) / 2, 13, size, size);
  roomKnob.setBounds(9, 52, size, size);
  dampingKnob.setBounds(getWidth() - size - 9, 52, size, size);
}

void ReverbPedalComponent::mouseDown(const juce::MouseEvent& event) {
  dragStarted = false;
  if (slotIndex.has_value() &&
      (event.mods.isPopupMenu() || event.mods.isCtrlDown())) {
    showContextMenu();
  }
}

void ReverbPedalComponent::mouseDrag(const juce::MouseEvent& event) {
  if (dragStarted || event.getDistanceFromDragStart() < 5) {
    return;
  }

  if (auto* container =
          juce::DragAndDropContainer::findParentDragContainerFor(this)) {
    dragStarted = true;
    container->startDragging(dragDescription(slotIndex), this);
  }
}

void ReverbPedalComponent::showContextMenu() {
  juce::PopupMenu menu;
  menu.addItem(deleteMenuId, "Delete Pedal");

  juce::PopupMenu moveMenu;
  for (auto slot = 0u; slot < pedalSlotCount; ++slot) {
    if (slot != *slotIndex) {
      moveMenu.addItem(moveMenuBaseId + static_cast<int>(slot),
                       "Slot " + juce::String{static_cast<int>(slot + 1)});
    }
  }
  menu.addSubMenu("Move Pedal", moveMenu);

  const auto safeThis = juce::Component::SafePointer{this};
  menu.showMenuAsync(
      juce::PopupMenu::Options{}.withTargetComponent(this),
      [safeThis](int result) {
        if (safeThis == nullptr) {
          return;
        }
        if (result == deleteMenuId) {
          safeThis->onDelete();
        } else if (result >= moveMenuBaseId &&
                   result < moveMenuBaseId +
                                static_cast<int>(pedalSlotCount)) {
          safeThis->onMove(
              static_cast<size_t>(result - moveMenuBaseId));
        }
      });
}

class PluginEditor::Impl : public juce::Component,
                           public juce::DragAndDropContainer,
                           private juce::ChangeListener {
public:
  explicit Impl(PluginProcessor& owner) : processor{owner} {
    processor.addChangeListener(this);

    pedalSelector.addItem("Reverb", 1);
    pedalSelector.setTextWhenNothingSelected("Select pedal");
    pedalSelector.onChange = [this] {
      paletteType = pedalSelector.getSelectedId() == 1
                        ? PedalType::reverb
                        : PedalType::empty;
      rebuildPalette();
    };
    addAndMakeVisible(pedalSelector);

    clearPaletteButton.setButtonText("x");
    clearPaletteButton.setTooltip("Clear selected pedal");
    clearPaletteButton.onClick = [this] {
      pedalSelector.setSelectedId(0);
      paletteType = PedalType::empty;
      rebuildPalette();
    };
    addAndMakeVisible(clearPaletteButton);

    bypassButton.setButtonText("Bypass");
    bypassAttachment = std::make_unique<juce::ButtonParameterAttachment>(
        processor.getParameterRefs().bypassed, bypassButton);
    addAndMakeVisible(bypassButton);

    for (auto slot = 0u; slot < pedalSlotCount; ++slot) {
      slots[slot] = std::make_unique<PedalSlotComponent>(
          slot, [this](size_t destination, const juce::var& description) {
            handleDrop(destination, description.toString());
          });
      addAndMakeVisible(*slots[slot]);
    }

    addAndMakeVisible(palettePanel);
    addAndMakeVisible(signalLabel);
    signalLabel.setText("INPUT  >  SLOT 1  >  ...  >  SLOT 10  >  OUTPUT",
                        juce::dontSendNotification);
    signalLabel.setJustificationType(juce::Justification::centred);
    signalLabel.setColour(juce::Label::textColourId,
                          juce::Colour{0xffb9bec7});

    syncFromProcessor();
  }

  ~Impl() override {
    processor.removeChangeListener(this);
  }

  void paint(juce::Graphics& g) override {
    g.fillAll(juce::Colour{0xff17191c});

    const auto board = getLocalBounds().reduced(18).toFloat();
    g.setColour(juce::Colour{0xff292c31});
    g.fillRoundedRectangle(board, 6.f);
    g.setColour(juce::Colour{0xff464b52});
    g.drawRoundedRectangle(board, 6.f, 2.f);

    g.setColour(juce::Colour{0xff9b5cff});
    g.setFont(juce::FontOptions{20.f}.withStyle("Bold"));
    g.drawText("PEDAL BOARD", 32, 25, 220, 30,
               juce::Justification::centredLeft);

    g.setColour(juce::Colour{0xffe4e6e9});
    g.setFont(juce::FontOptions{42.f});
    g.drawText("PedalBoard", 32, getHeight() - 72, 300, 50,
               juce::Justification::centredLeft);
  }

  void resized() override {
    auto content = getLocalBounds().reduced(32);
    auto header = content.removeFromTop(54);
    header.removeFromLeft(235);
    bypassButton.setBounds(header.removeFromRight(88).reduced(5, 9));
    clearPaletteButton.setBounds(header.removeFromRight(38).reduced(4, 9));
    pedalSelector.setBounds(header.removeFromRight(225).reduced(4, 9));

    content.removeFromBottom(72);
    auto paletteArea = content.removeFromRight(180);
    palettePanel.setBounds(paletteArea.reduced(12, 4));

    auto signalArea = content.removeFromBottom(30);
    signalLabel.setBounds(signalArea);

    const auto gap = 10;
    const auto rowGap = 12;
    const auto slotWidth = (content.getWidth() - gap * 4) / 5;
    const auto slotHeight = (content.getHeight() - rowGap) / 2;
    for (auto slot = 0u; slot < pedalSlotCount; ++slot) {
      const auto column = static_cast<int>(slot % 5);
      const auto row = static_cast<int>(slot / 5);
      slots[slot]->setBounds(content.getX() + column * (slotWidth + gap),
                             content.getY() + row * (slotHeight + rowGap),
                             slotWidth, slotHeight);
    }

    if (palettePedal != nullptr) {
      palettePedal->setBounds(palettePanel.getLocalBounds().reduced(20, 46));
    }
  }

private:
  void changeListenerCallback(juce::ChangeBroadcaster*) override {
    syncFromProcessor();
  }

  void handleDrop(size_t destination, const juce::String& description) {
    if (processor.getPedal(destination) != PedalType::empty) {
      return;
    }

    if (description == "new:reverb") {
      processor.addPedal(destination, PedalType::reverb);
      return;
    }

    if (const auto source = sourceSlotFromDescription(description)) {
      processor.movePedal(*source, destination);
    }
  }

  void syncFromProcessor() {
    for (auto slot = 0u; slot < pedalSlotCount; ++slot) {
      slots[slot]->removeAllChildren();
      slotPedals[slot].reset();

      if (processor.getPedal(slot) == PedalType::reverb) {
        slotPedals[slot] = std::make_unique<ReverbPedalComponent>(
            processor.getParameterRefs(), slot,
            [this, slot] { processor.deletePedal(slot); },
            [this, slot](size_t destination) {
              processor.movePedal(slot, destination);
            });
        slots[slot]->addAndMakeVisible(*slotPedals[slot]);
        slots[slot]->resized();
      }
    }
    repaint();
  }

  void rebuildPalette() {
    palettePanel.removeAllChildren();
    palettePedal.reset();
    if (paletteType == PedalType::reverb) {
      palettePedal = std::make_unique<ReverbPedalComponent>(
          processor.getParameterRefs(), std::nullopt, [] {}, [](size_t) {});
      palettePanel.addAndMakeVisible(*palettePedal);
      resized();
    }
    palettePanel.repaint();
  }

  PluginProcessor& processor;
  juce::ComboBox pedalSelector;
  juce::TextButton clearPaletteButton;
  juce::ToggleButton bypassButton;
  std::unique_ptr<juce::ButtonParameterAttachment> bypassAttachment;
  juce::Component palettePanel;
  juce::Label signalLabel;
  PedalType paletteType = PedalType::empty;
  std::unique_ptr<ReverbPedalComponent> palettePedal;
  std::array<std::unique_ptr<PedalSlotComponent>, pedalSlotCount> slots;
  std::array<std::unique_ptr<ReverbPedalComponent>, pedalSlotCount>
      slotPedals;
};

PluginEditor::PluginEditor(PluginProcessor& owner)
    : AudioProcessorEditor{&owner},
      impl{std::make_unique<Impl>(owner)} {
  addAndMakeVisible(*impl);
  setResizable(true, true);
  setResizeLimits(760, 500, 1400, 900);
  setSize(1040, 650);
}

PluginEditor::~PluginEditor() = default;

void PluginEditor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colour{0xff17191c});
}

void PluginEditor::resized() {
  impl->setBounds(getLocalBounds());
}
}  // namespace pedalboard
