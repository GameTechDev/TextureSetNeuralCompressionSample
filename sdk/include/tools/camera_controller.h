/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

// SDK includes
#include "graphics/types.h"
#include "graphics/event_collector.h"

// System includes
#include <string>

struct Camera
{
    // Projection parameters
    float fov;
    float2 nearFar;
    float aspectRatio;

    // View parameters
    float3 position;
    float3 angles;

    // Zoom parameters
    float3 scaleOffset;

    // Transformation matrices
    float4x4 view;
    float4x4 projection;

    // Compound matrices
    float4x4 viewProjection;
    float4x4 invViewProjection;
};

struct Transform
{
    float4 rotation;
    float3 position;
    float fov;
    float3 angles;
};

// Buttons that control the camera movements
enum class NavigationButtons
{
    Forward = 0,
    Backward,
    Left,
    Right,
    Up,
    Down,
    Shift,
    Count
};

class CameraController
{
public:
    CameraController();
    ~CameraController();

    // Initialize the controller
    virtual void initialize(RenderWindow window, uint32_t width, uint32_t height, float fov, const std::string& pathDir);

    // Render the UI
    virtual void render_ui();

    // Process key event
    virtual void process_key_event(uint32_t keyCode, bool state);

    // Process a mouse mouvement
    virtual bool process_mouse_movement(int2 mouse, uint2 windowCenter, float4 screenSize);
    virtual void process_mouse_wheel(int wheel);
    virtual bool process_mouse_button(MouseButton button, bool state);

    // Apply the delta time
    virtual void update(double deltaTime);
    virtual void evaluate_camera_matrices();

    // Get camera
    const Camera& get_camera() const { return m_Camera; }
    Camera& get_camera() { return m_Camera; }

    // Update the camera matrices
    void setup_play_path(uint32_t targetPoint);
    void load_camera_path(const char* pathName);
    void save_camera_path(const char* pathName);
    void move_to_poi(uint32_t poiIDX);

protected:
    // Render window
    RenderWindow m_Window = 0;

    // The camera that the controller is handeling
    Camera m_Camera = Camera();
    
    // Button controls
    bool m_ControllerStates[(uint32_t)NavigationButtons::Count] = { false, false, false, false, false, false, false };

    // Flag that defines if we can interact with the camera
    bool m_ActiveInteraction = false;
    std::string m_PathsDir;

    // Control points
    std::vector<Transform> m_POIArray = {};

    // Is playing
    bool m_IsPlaying = false;
    float m_PlayTime = 0.0f;
    float m_Duration = 2.0f;
    float3 m_PositionSpline[2] = { 0.0f,0.0f,0.0f };
    float4 m_RotationSpline[2] = { 0.0f,0.0f,0.0f, 0.0f };
    float m_FOVSpline[2] = { 0.0f, 0.0f };
    uint32_t m_TargetPOI = UINT32_MAX;

    // Speed
    float m_Speed = 0.0f;
};
