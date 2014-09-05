// Copyright (c) 2014 Google, Inc.
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#define FPL_AUDIO_ENGINE_UNIT_TESTS

#include <vector>
#include "audio_engine.h"
#include "gtest/gtest.h"
#include "sound_generated.h"
#include "SDL_mixer.h"
#include "sound.h"

// Stubs for SDL_mixer functions which are not actually part of the tests being
// run.
extern "C" {
void Mix_CloseAudio() {}
int Mix_OpenAudio(int, Uint16, int, int) { return 0; }
int Mix_AllocateChannels(int) { return 0; }
int Mix_Playing(int) { return 0; }
int Mix_HaltChannel(int) { return 0; }
int Mix_PlayChannelTimed(int, Mix_Chunk*, int, int) { return 0; }
const char* SDL_GetError(void) { return NULL; }
void Mix_FreeChunk(Mix_Chunk*) {}
Mix_Chunk* Mix_LoadWAV_RW(SDL_RWops*, int) { return NULL; }
}

namespace fpl {

bool LoadFile(const char*, std::string*) { return false; }

class AudioEngineTests : public ::testing::Test {
 protected:
  virtual void SetUp() {
    // Make a bunch of sound defs with various priorities.
    for (int i = 0; i < 6; ++i) {
      flatbuffers::FlatBufferBuilder builder;
      int id = i;
      float priority = static_cast<float>(i);
      auto sound_def_buffer = fpl::CreateSoundDef(builder, id, priority);
      builder.Finish(sound_def_buffer);
      sounds_.push_back(fpl::Sound());
      sounds_.back().LoadSound(
          std::string(reinterpret_cast<const char*>(builder.GetBufferPointer()),
                      builder.GetSize()));
    }
  }
  virtual void TearDown() {}

 protected:
  std::vector<fpl::Sound> sounds_;
};

TEST_F(AudioEngineTests, IncreasingPriority) {
  std::vector<fpl::AudioEngine::PlayingSound> playing_sounds;
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(0, 0, 0));
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(1, 1, 1));
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(2, 2, 2));
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(3, 3, 3));
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(4, 4, 4));
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(5, 5, 5));
  fpl::AudioEngine::PrioritizeChannels(sounds_, &playing_sounds);
  EXPECT_EQ(0, playing_sounds[5].channel_id);
  EXPECT_EQ(1, playing_sounds[4].channel_id);
  EXPECT_EQ(2, playing_sounds[3].channel_id);
  EXPECT_EQ(3, playing_sounds[2].channel_id);
  EXPECT_EQ(4, playing_sounds[1].channel_id);
  EXPECT_EQ(5, playing_sounds[0].channel_id);
}

TEST_F(AudioEngineTests, SamePriorityDifferentStartTimes) {
  std::vector<fpl::AudioEngine::PlayingSound> playing_sounds;
  // Sounds with the same priority but later start times should be higher
  // priority.
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(0, 0, 1));
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(0, 1, 0));
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(1, 2, 1));
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(1, 3, 0));
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(2, 4, 1));
  playing_sounds.push_back(fpl::AudioEngine::PlayingSound(2, 5, 0));
  fpl::AudioEngine::PrioritizeChannels(sounds_, &playing_sounds);
  EXPECT_EQ(0, playing_sounds[5].channel_id);
  EXPECT_EQ(1, playing_sounds[4].channel_id);
  EXPECT_EQ(2, playing_sounds[3].channel_id);
  EXPECT_EQ(3, playing_sounds[2].channel_id);
  EXPECT_EQ(4, playing_sounds[1].channel_id);
  EXPECT_EQ(5, playing_sounds[0].channel_id);
}

}  // namespace fpl

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
