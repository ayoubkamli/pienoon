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
#include <vector>
#include "character_state_machine_def_generated.h"
#include "common.h"
#include "controller.h"
#include "gamepad_controller.h"
#include "timeline_generated.h"

namespace fpl {
namespace pie_noon {

static const fplbase::AndroidInputDeviceId kInvalidControllerId = -1;

GamepadController::GamepadController()
    : Controller(kTypeGamepad),
      input_system_(nullptr),
      controller_id_(kInvalidControllerId) {}

void GamepadController::Initialize(fplbase::InputSystem* input_system,
                                   fplbase::AndroidInputDeviceId controller_id) {
  input_system_ = input_system;
  controller_id_ = controller_id;
  ClearAllLogicalInputs();
}

// How far an analog stick has to be tilted before we count it.
static const float kAnalogDeadZone = 0.25f;

void GamepadController::AdvanceFrame(WorldTime /*delta_time*/) {
  went_down_ = went_up_ = 0;
  fplbase::Gamepad gamepad = input_system_->GetGamepad(controller_id_);
  SetLogicalInputs(LogicalInputs_Up,
                   gamepad.GetButton(fplbase::Gamepad::kUp).is_down());
  SetLogicalInputs(LogicalInputs_Down,
                   gamepad.GetButton(fplbase::Gamepad::kDown).is_down());
  SetLogicalInputs(LogicalInputs_Left,
                   gamepad.GetButton(fplbase::Gamepad::kLeft).is_down());
  SetLogicalInputs(LogicalInputs_Right,
                   gamepad.GetButton(fplbase::Gamepad::kRight).is_down());

  SetLogicalInputs(LogicalInputs_ThrowPie,
                   gamepad.GetButton(fplbase::Gamepad::kUp).is_down() ||
                       gamepad.GetButton(fplbase::Gamepad::kButtonA).is_down());
  SetLogicalInputs(LogicalInputs_Deflect,
                   gamepad.GetButton(fplbase::Gamepad::kDown).is_down() ||
                       gamepad.GetButton(fplbase::Gamepad::kButtonB).is_down());

  SetLogicalInputs(LogicalInputs_Select,
                   gamepad.GetButton(fplbase::Gamepad::kButtonA).is_down());
  SetLogicalInputs(LogicalInputs_Cancel,
                   gamepad.GetButton(fplbase::Gamepad::kButtonB).is_down());
}

}  // pie_noon
}  // fpl
