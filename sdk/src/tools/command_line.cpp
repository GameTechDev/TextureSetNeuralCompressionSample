/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */
 
 // Includes
#include "tools/command_line.h"

namespace command_line
{
	bool parse_args(CommandLineOptions& commandLineOptions, const std::vector<std::string>& args)
	{
		// Path of the exe
		uint32_t loc = (uint32_t)args[0].find_last_of('\\');
		std::string exeDir = (loc == UINT32_MAX) ? "." : args[0].substr(0, loc);
		commandLineOptions.dataDir = exeDir;

		uint32_t current_arg_idx = 1;
		const uint32_t num_args = (uint32_t)args.size();
		while(current_arg_idx < num_args)
		{
			if(args[current_arg_idx] == "--data-dir")
			{
				if(current_arg_idx == num_args - 1)
				{
					printf("Command line parser: please provide a data directory.");
					continue;
				}
				commandLineOptions.dataDir = args[current_arg_idx + 1];
				current_arg_idx += 2;
			}
			else if(args[current_arg_idx] == "--adapter-id")
			{
				if(current_arg_idx == num_args - 1)
				{
					printf("Command line parser: please provide an adapter ID.");
					continue;
				}
				commandLineOptions.adapterIndex = atoi(args[current_arg_idx + 1].c_str());
				current_arg_idx += 2;
			}
			else if (args[current_arg_idx] == "--poi")
			{
				if (current_arg_idx == num_args - 1)
				{
					printf("Command line parser: please provide a POI ID.");
					continue;
				}
				commandLineOptions.initialPOI = atoi(args[current_arg_idx + 1].c_str());
				current_arg_idx += 2;
			}
			else if (args[current_arg_idx] == "--disable-coop")
			{
				commandLineOptions.enableCooperative = false;
				current_arg_idx += 1;
			}
			else if (args[current_arg_idx] == "--enable-animation")
			{
				commandLineOptions.enableAnimation = true;
				current_arg_idx += 1;
			}
			else if (args[current_arg_idx] == "--help")
			{
				printf("Option list:\n");
				printf("--data-dir Location of the resource folders.\n");
				printf("--adapter-id Integer that allows to pick the desired GPU.\n");
				printf("--poi Integer that allows to pick the initial camera location.\n");
				printf("--disable-coop Disable cooperative vector usage at launch.\n");
				printf("--enable-animation Enable mesh animation at launch.\n");
				return false;
			}
			else
			{
				printf("Command line parser: unrecognized compiler option.");
				current_arg_idx++;
			}
		}
		return true;
	}
}
