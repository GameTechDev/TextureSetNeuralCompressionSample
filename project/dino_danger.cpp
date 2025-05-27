/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "render_pipeline/dino_renderer.h"
#include "tools/command_line.h"

// System includes
#define NOMINMAX
#include <Windows.h>

int CALLBACK main(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    // Stack the command line args
    std::vector<std::string> args;
    for (uint32_t idx = 0; idx < (uint32_t)__argc; ++idx)
        args.push_back(__argv[idx]);

    // Parse the command line
    CommandLineOptions options;
    if (!command_line::parse_args(options, args))
        return -1;
    
    // Create the renderer
    DinoRenderer renderer;

    // Initalize the renderer
    renderer.initialize((uint64_t)hInstance, options);

    // Trigger the renderer loop
    renderer.render_loop();

    // Release all the resources
    renderer.release();

    // We're done
    return 0;
}