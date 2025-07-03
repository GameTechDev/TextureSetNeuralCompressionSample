/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// Project includes
#include "render_pipeline/types.h"

// System includes
#include <vector>
#include <string>

struct CommandLineOptions
{
	// Location of the data
	std::string dataDir = ".";

	// Adapter index (as returned by OS)
	int32_t adapterIndex = -1;

	// Picking an initial POI
	uint32_t initialPOI = 0;

	// Cooperative vectors enabled at start
	bool enableCooperative = true;

	// Mesh animation enabled at start
	bool disableAnimation = false;

	// Control the rendering mode
	RenderingMode renderingMode = RenderingMode::GBufferDeferred;

	// Control the texture mode
	TextureMode textureMode = TextureMode::Neural;

	// Filtering mode
	FilteringMode filteringMode = FilteringMode::Anisotropic;
};

namespace command_line
{
	bool parse_args(CommandLineOptions& commandLineOptions, const std::vector<std::string>& args);
}
