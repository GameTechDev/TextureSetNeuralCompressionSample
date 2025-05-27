/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// System includes
#include <vector>
#include <string>

struct CommandLineOptions
{
	// Location of the data
	std::string dataDir = ".";

	// Adapter index (as returned by OS)
	uint32_t adapterIndex = 0;

	// Picking an initial POI
	uint32_t initialPOI = 0;

	// Cooperative vectors enabled at start
	bool enableCooperative = true;

	// Mesh animation enabled at start
	bool enableAnimation = true;
};

namespace command_line
{
	bool parse_args(CommandLineOptions& commandLineOptions, const std::vector<std::string>& args);
}
