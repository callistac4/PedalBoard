namespace pedalboard {
PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {}

const juce::String PluginProcessor::getName() const {
  return PEDALBOARD_PLUGIN_NAME;
}

bool PluginProcessor::acceptsMidi() const {
  return false;
}

bool PluginProcessor::producesMidi() const {
  return false;
}

bool PluginProcessor::isMidiEffect() const {
  return false;
}

double PluginProcessor::getTailLengthSeconds() const {
  return 8.0;
}

int PluginProcessor::getNumPrograms() {
  return 1;
}

int PluginProcessor::getCurrentProgram() {
  return 0;
}

void PluginProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return "None";
}

void PluginProcessor::changeProgramName(int index, const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

void PluginProcessor::prepareToPlay(double sampleRate, int expectedMaxFramesPerBlock) {
  juce::dsp::ProcessSpec spec;
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = static_cast<juce::uint32>(expectedMaxFramesPerBlock);
  spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

  currentSampleRate.store(sampleRate);
  pedalboard.prepare(sampleRate, expectedMaxFramesPerBlock,
                     getTotalNumOutputChannels());
  bypassTransitionSmoother.prepare(spec);
}

void PluginProcessor::releaseResources() {
  pedalboard.reset();
  bypassTransitionSmoother.reset();
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
    return false;
  }
  // This checks if the input layout matches the output layout
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) {
    return false;
  }
  return true;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);

  juce::ScopedNoDenormals noDenormals;
  const auto totalNumInputChannels = getTotalNumInputChannels();
  const auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (const auto channelToClear :
       std::views::iota(totalNumInputChannels, totalNumOutputChannels)) {
    buffer.clear(channelToClear, 0, buffer.getNumSamples());
  }

  const auto bypassedAndNotTransitioning =
      parameters.bypassed.get() && !bypassTransitionSmoother.isTransitioning();

  bypassTransitionSmoother.setBypass(parameters.bypassed);

  // avoid processing if the plugin is fully bypassed
  if (bypassedAndNotTransitioning) { return; }

  bypassTransitionSmoother.setDryBuffer(buffer);

  pedalboard.process(
      buffer,
      {.roomSize = parameters.reverbRoomSize.get(),
       .damping = parameters.reverbDamping.get(),
       .mix = parameters.reverbMix.get(),
       .width = parameters.reverbWidth.get()});

  bypassTransitionSmoother.mixToWetBuffer(buffer);
}

bool PluginProcessor::hasEditor() const {
  return true;
}

// This function will be called to create an instance of the editor
juce::AudioProcessorEditor* PluginProcessor::createEditor() {
  return new PluginEditor(*this);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock& destData) {
  juce::MemoryOutputStream outputStream{destData, true};
  JsonSerializer::serialize(parameters, pedalboard.getPedals(), outputStream);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes) {
  juce::MemoryInputStream inputStream{data, static_cast<size_t>(sizeInBytes),
                                      false};
  auto restoredPedals = pedalboard.getPedals();
  const auto result =
      JsonSerializer::deserialize(inputStream, parameters, restoredPedals);

  if (result.failed()) {
    DBG(result.getErrorMessage());
    return;
  }

  bypassTransitionSmoother.setBypassForced(parameters.bypassed);
  pedalboard.setPedals(restoredPedals);
  sendChangeMessage();
}

Parameters& PluginProcessor::getParameterRefs() noexcept {
  return parameters;
}

juce::AudioProcessorParameter* PluginProcessor::getBypassParameter()
    const noexcept {
  return &parameters.bypassed;
}

PedalType PluginProcessor::getPedal(size_t slot) const noexcept {
  return pedalboard.getPedal(slot);
}

std::array<PedalType, pedalSlotCount> PluginProcessor::getPedals()
    const noexcept {
  return pedalboard.getPedals();
}

bool PluginProcessor::addPedal(size_t slot, PedalType type) {
  if (slot >= pedalSlotCount || type == PedalType::empty ||
      pedalboard.getPedal(slot) != PedalType::empty) {
    return false;
  }

  pedalboard.setPedal(slot, type);
  sendChangeMessage();
  return true;
}

bool PluginProcessor::movePedal(size_t source, size_t destination) {
  const auto moved = pedalboard.movePedal(source, destination);
  if (moved) {
    sendChangeMessage();
  }
  return moved;
}

void PluginProcessor::deletePedal(size_t slot) {
  if (slot < pedalSlotCount &&
      pedalboard.getPedal(slot) != PedalType::empty) {
    pedalboard.setPedal(slot, PedalType::empty);
    sendChangeMessage();
  }
}

double PluginProcessor::getSampleRateThreadSafe() const noexcept {
  return currentSampleRate;
}
}  // namespace pedalboard

// This creates new instances of the plugin. This function definition must be in the global namespace.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new pedalboard::PluginProcessor();
}
