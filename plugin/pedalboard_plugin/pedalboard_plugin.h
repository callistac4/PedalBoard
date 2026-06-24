/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            pedalboard_plugin
   vendor:        Callista Sound
   version:       1.0.0
   name:          PedalBoard Plugin
   description:   Core of the pedalboard plugin
   dependencies:  juce_audio_utils, juce_dsp

   website:       https://github.com/callistachong/PedalBoard
   license:       MIT

END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <memory>
#include <functional>
#include <ranges>
#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <span>
#include <optional>
#include "include/PedalBoard/detail/StridedQueue.h"

#include "include/PedalBoard/PedalTypes.h"
#include "include/PedalBoard/ReverbProcessor.h"
#include "include/PedalBoard/Parameters.h"
#include "include/PedalBoard/JsonSerializer.h"
#include "include/PedalBoard/PedalBoard.h"
#include "include/PedalBoard/BypassTransitionSmoother.h"
#include "include/PedalBoard/PluginProcessor.h"
#include "include/PedalBoard/PluginEditor.h"
#include "include/PedalBoard/ReverbPedalComponent.h"
