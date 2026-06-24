namespace pedalboard {
namespace {
constexpr auto stateVersion = 2;

struct DeserializedState {
  bool bypassed = false;
  float roomSize = 0.55f;
  float damping = 0.45f;
  float mix = 0.3f;
  float width = 1.f;
  std::array<PedalType, pedalSlotCount> pedals{};
};

juce::Result readFloat(const juce::DynamicObject& object,
                       const juce::Identifier& property,
                       float& destination) {
  if (!object.hasProperty(property)) {
    return juce::Result::fail("missing state property: " +
                              property.toString());
  }

  const auto value = static_cast<float>(object.getProperty(property));
  if (!std::isfinite(value) || value < 0.f || value > 1.f) {
    return juce::Result::fail("invalid state property: " +
                              property.toString());
  }

  destination = value;
  return juce::Result::ok();
}
}  // namespace

void JsonSerializer::serialize(
    const Parameters& parameters,
    const std::array<PedalType, pedalSlotCount>& pedals,
    juce::OutputStream& output) {
  auto root = std::make_unique<juce::DynamicObject>();
  root->setProperty("__version__", stateVersion);
  root->setProperty("pluginName", PEDALBOARD_PLUGIN_NAME);
  root->setProperty("bypassed", parameters.bypassed.get());
  root->setProperty("reverbRoomSize", parameters.reverbRoomSize.get());
  root->setProperty("reverbDamping", parameters.reverbDamping.get());
  root->setProperty("reverbMix", parameters.reverbMix.get());
  root->setProperty("reverbWidth", parameters.reverbWidth.get());

  juce::Array<juce::var> serializedPedals;
  for (const auto pedal : pedals) {
    serializedPedals.add(pedalTypeToString(pedal));
  }
  root->setProperty("pedals", serializedPedals);

  juce::JSON::writeToStream(
      output, juce::var{root.release()},
      juce::JSON::FormatOptions{}
          .withSpacing(juce::JSON::Spacing::multiLine)
          .withMaxDecimalPlaces(3));
}

juce::Result JsonSerializer::deserialize(
    juce::InputStream& input,
    Parameters& parameters,
    std::array<PedalType, pedalSlotCount>& pedals) {
  juce::var parsed;
  const auto parseResult =
      juce::JSON::parse(input.readEntireStreamAsString(), parsed);
  if (parseResult.failed()) {
    return parseResult;
  }

  const auto* root = parsed.getDynamicObject();
  if (root == nullptr) {
    return juce::Result::fail("plugin state must be a JSON object");
  }
  if (static_cast<int>(root->getProperty("__version__")) != stateVersion) {
    return juce::Result::fail("unsupported plugin state version");
  }
  if (root->getProperty("pluginName").toString() != PEDALBOARD_PLUGIN_NAME) {
    return juce::Result::fail("plugin state belongs to a different plugin");
  }

  DeserializedState state;
  state.bypassed = static_cast<bool>(root->getProperty("bypassed"));

  for (const auto& [property, destination] :
       {std::pair{juce::Identifier{"reverbRoomSize"}, &state.roomSize},
        std::pair{juce::Identifier{"reverbDamping"}, &state.damping},
        std::pair{juce::Identifier{"reverbMix"}, &state.mix},
        std::pair{juce::Identifier{"reverbWidth"}, &state.width}}) {
    const auto result = readFloat(*root, property, *destination);
    if (result.failed()) {
      return result;
    }
  }

  const auto* serializedPedals = root->getProperty("pedals").getArray();
  if (serializedPedals == nullptr ||
      serializedPedals->size() != static_cast<int>(pedalSlotCount)) {
    return juce::Result::fail("plugin state must contain exactly 10 slots");
  }

  for (auto slot = 0u; slot < pedalSlotCount; ++slot) {
    const auto type =
        pedalTypeFromString(serializedPedals->getReference(
                                static_cast<int>(slot))
                                .toString());
    if (!type.has_value()) {
      return juce::Result::fail("plugin state contains an unknown pedal");
    }
    state.pedals[slot] = *type;
  }

  parameters.bypassed = state.bypassed;
  parameters.reverbRoomSize = state.roomSize;
  parameters.reverbDamping = state.damping;
  parameters.reverbMix = state.mix;
  parameters.reverbWidth = state.width;
  pedals = state.pedals;
  return juce::Result::ok();
}
}  // namespace pedalboard
