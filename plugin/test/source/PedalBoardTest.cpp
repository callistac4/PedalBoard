#include <gtest/gtest.h>
#include <pedalboard_plugin/pedalboard_plugin.h>

namespace pedalboard {
TEST(PedalBoard, StartsWithTenEmptySlots) {
  PedalBoard board;

  for (const auto pedal : board.getPedals()) {
    EXPECT_EQ(PedalType::empty, pedal);
  }
}

TEST(PedalBoard, MovesPedalsOnlyIntoEmptySlots) {
  PedalBoard board;
  board.setPedal(2, PedalType::reverb);
  board.setPedal(5, PedalType::reverb);

  EXPECT_FALSE(board.movePedal(2, 5));
  EXPECT_EQ(PedalType::reverb, board.getPedal(2));

  EXPECT_TRUE(board.movePedal(2, 7));
  EXPECT_EQ(PedalType::empty, board.getPedal(2));
  EXPECT_EQ(PedalType::reverb, board.getPedal(7));
}

TEST(PedalBoard, EmptyBoardLeavesAudioUnchanged) {
  PedalBoard board;
  board.prepare(48000.0, 128, 2);

  juce::AudioBuffer<float> buffer{2, 128};
  buffer.clear();
  buffer.setSample(0, 0, 1.f);
  buffer.setSample(1, 0, -1.f);
  const auto original = buffer;

  board.process(buffer, ReverbSettings{});

  for (auto channel = 0; channel < buffer.getNumChannels(); ++channel) {
    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample) {
      EXPECT_FLOAT_EQ(original.getSample(channel, sample),
                      buffer.getSample(channel, sample));
    }
  }
}

TEST(PedalBoard, ReverbSlotProcessesAudio) {
  PedalBoard board;
  board.setPedal(0, PedalType::reverb);
  board.prepare(48000.0, 4096, 1);

  juce::AudioBuffer<float> buffer{1, 4096};
  buffer.clear();
  buffer.setSample(0, 0, 1.f);

  board.process(buffer, {.roomSize = 0.8f,
                         .damping = 0.4f,
                         .mix = 0.5f,
                         .width = 1.f});

  EXPECT_NE(1.f, buffer.getSample(0, 0));
  float tailMagnitude = 0.f;
  for (auto sample = 1; sample < buffer.getNumSamples(); ++sample) {
    tailMagnitude += std::abs(buffer.getSample(0, sample));
  }
  EXPECT_GT(tailMagnitude, 0.f);
}
}  // namespace pedalboard
