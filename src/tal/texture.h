#pragma once

#ifdef SOTA_GDEXTENSION
#include "godot_cpp/classes/texture.hpp"

using Texture = godot::Texture;
#else
#include "scene/resources/texture.h"

using Texture = Texture;
#endif
