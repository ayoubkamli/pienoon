// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "precompiled.h"
#include "sound.h"
#include "SDL_mixer.h"
#include "utilities.h"
#include "sound_generated.h"

namespace fpl {

Sample::~Sample() {
  if (chunk_) {
    Mix_FreeChunk(chunk_);
  }
}

bool Sample::LoadSample(const char* filename) {
  chunk_ = Mix_LoadWAV(filename);
  if (!chunk_) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "can't load %s\n", filename);
    return false;
  }
  return true;
}

static int LoadSamples(const SoundDef* def,
                       std::vector<Sample>* samples,
                       float* sum_of_probabilities) {
  unsigned int sample_count =
      def->audio_sample_set() ? def->audio_sample_set()->Length() : 0;
  samples->resize(sample_count);
  for (unsigned int i = 0; i < sample_count; ++i) {
    const AudioSampleSetEntry* entry = def->audio_sample_set()->Get(i);
    const char* entry_filename = entry->audio_sample()->filename()->c_str();
    Sample& sample = (*samples)[i];
    if (!sample.LoadSample(entry_filename)) {
      return false;
    }
    *sum_of_probabilities += entry->playback_probability();
  }
  return true;
}

bool Sound::LoadSound(const std::string& sound_def_source) {
  source_ = sound_def_source;
  return LoadSamples(GetSoundDef(), &samples_, &sum_of_probabilities_);
}

bool Sound::LoadSoundFromFile(const char* filename) {
  if (!LoadFile(filename, &source_)) {
    return false;
  }
  return LoadSamples(GetSoundDef(), &samples_, &sum_of_probabilities_);
}

void Sound::Unload() {
  source_.clear();
  samples_.clear();
  sum_of_probabilities_ = 0;
}

const SoundDef* Sound::GetSoundDef() const {
  assert(source_.size());
  return fpl::GetSoundDef(source_.c_str());
}

Mix_Chunk* Sound::SelectChunk() {
  const SoundDef* sound_def = GetSoundDef();

  // Choose a random number between 0 and the sum of the probabilities, then
  // iterate over the list, subtracting the weight of each entry until 0 is
  // reached.
  float selection = mathfu::Random<float>() * sum_of_probabilities_;
  for (unsigned int i = 0; i < samples_.size(); ++i) {
    const AudioSampleSetEntry* entry = sound_def->audio_sample_set()->Get(i);
    selection -= entry->playback_probability();
    if (selection <= 0) {
      return samples_[i].chunk();
    }
  }

  // If we've reached here and didn't return a sound, assume there was some
  // floating point rounding error and just return the last one.
  return samples_.back().chunk();
}

}  // namespace fpl

