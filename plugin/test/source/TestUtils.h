#pragma once
#include <juce_core/juce_core.h>

namespace pedalboard {
inline std::string getFileOutputPath(juce::StringRef fileName) {
  return juce::File::getSpecialLocation(
             juce::File::SpecialLocationType::currentExecutableFile)
      .getParentDirectory()
      .getChildFile(fileName)
      .getFullPathName()
      .toStdString();
}
}  // namespace tremolo
