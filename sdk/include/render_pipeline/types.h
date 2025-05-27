/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

enum class DebugMode
{
	Thickness = 0,
	Mask,
	Displacement,
	Metalness,
	Roughness,
	AmbientOcclusion,
	Normal,
	DiffuseColor,
	TileInfo,
	Count
};

enum class RenderingMode
{
	MaterialPass = 0,
	GBufferDeferred,
	Debug,
	Count
};

enum class TextureMode
{
	Uncompressed = 0,
	BC6H,
	Neural,
	Count
};

enum class InferenceMode
{
	FMA = 0,
	CoopVector,
	Count
};

enum class FilteringMode
{
	Nearest = 0,
	Linear,
	Anisotropic,
	Count
};