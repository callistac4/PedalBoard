#include <gtest/gtest.h>
#include <pedalboard_plugin/pedalboard_plugin.h>

namespace pedalboard {
TEST(JsonSerializer, RoundTripsParametersAndPedals) {
  PluginProcessor sourceProcessor;
  auto& sourceParameters = sourceProcessor.getParameterRefs();
  sourceParameters.bypassed = true;
  sourceParameters.reverbRoomSize = 0.72f;
  sourceParameters.reverbDamping = 0.36f;
  sourceParameters.reverbMix = 0.41f;
  sourceParameters.reverbWidth = 0.83f;

  std::array<PedalType, pedalSlotCount> sourcePedals{};
  sourcePedals[1] = PedalType::reverb;
  sourcePedals[8] = PedalType::reverb;

  juce::MemoryBlock block;
  juce::MemoryOutputStream output{block, false};
  JsonSerializer::serialize(sourceParameters, sourcePedals, output);
  output.flush();

  PluginProcessor destinationProcessor;
  auto& destinationParameters = destinationProcessor.getParameterRefs();
  std::array<PedalType, pedalSlotCount> destinationPedals{};
  juce::MemoryInputStream input{block, false};
  const auto result = JsonSerializer::deserialize(
      input, destinationParameters, destinationPedals);

  EXPECT_TRUE(result.wasOk());
  EXPECT_TRUE(destinationParameters.bypassed.get());
  EXPECT_FLOAT_EQ(0.72f, destinationParameters.reverbRoomSize.get());
  EXPECT_FLOAT_EQ(0.36f, destinationParameters.reverbDamping.get());
  EXPECT_FLOAT_EQ(0.41f, destinationParameters.reverbMix.get());
  EXPECT_FLOAT_EQ(0.83f, destinationParameters.reverbWidth.get());
  EXPECT_EQ(sourcePedals, destinationPedals);
}

TEST(JsonSerializer, RejectsUnknownPedalWithoutChangingState) {
  const juce::String invalidState =
      R"json({
        "__version__": 2,
        "pluginName": "Pedal Board",
        "bypassed": true,
        "reverbRoomSize": 0.7,
        "reverbDamping": 0.4,
        "reverbMix": 0.3,
        "reverbWidth": 1.0,
        "pedals": [
          "Fuzz", "Empty", "Empty", "Empty", "Empty",
          "Empty", "Empty", "Empty", "Empty", "Empty"
        ]
      })json";

  PluginProcessor processor;
  auto& parameters = processor.getParameterRefs();
  std::array<PedalType, pedalSlotCount> pedals{};
  juce::MemoryInputStream input{
      invalidState.getCharPointer(),
      static_cast<size_t>(invalidState.getNumBytesAsUTF8()), false};

  const auto result = JsonSerializer::deserialize(input, parameters, pedals);

  EXPECT_TRUE(result.failed());
  EXPECT_FALSE(parameters.bypassed.get());
  EXPECT_EQ(PedalType::empty, pedals[0]);
}
}  // namespace pedalboard
