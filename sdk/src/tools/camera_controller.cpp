/*
 * Copyright (C) 2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Includes
#include "graphics/backend.h"
#include "imgui/imgui.h"
#include "math/operators.h"
#include "tools/camera_controller.h"
#include "tools/security.h"

// System includes
#include <algorithm>
#include <sstream>
#include <fstream>

CameraController::CameraController()
{
}

CameraController::~CameraController()
{
}

void CameraController::initialize(RenderWindow renderWindow, uint32_t width, uint32_t height, float fov, const std::string& pathDir)
{
    // Keep track of the window
    m_Window = renderWindow;

    // Camera properties
    m_Camera.fov = fov;
    m_Camera.nearFar = { 0.01, 100.0 };
    m_Camera.aspectRatio = width / (float)height;
    m_Camera.position = {-4, -3, 3.4};
    m_Camera.angles = {-2.2, 0.42, 0.0 };
    m_Speed = 10.0f;

    // Init control
    m_ActiveInteraction = false;
    m_PathsDir = pathDir + "\\";

    load_camera_path("poi_list.csv");
}

void CameraController::render_ui()
{
    int32_t numPOI = (int32_t)m_POIArray.size();
    float totalSize = 300.0f + numPOI * 23.0f;
    ImGui::SetNextWindowSize(ImVec2(550.0f, totalSize));

    ImGui::SeparatorText("Camera Properties");
    for (int32_t poiIdx = 0; poiIdx < numPOI - 3; ++poiIdx)
    {
        std::string label = "POI ";
        label += std::to_string(poiIdx);
        if (ImGui::Button(label.c_str()))
            setup_play_path(poiIdx);
        if ((poiIdx) != numPOI - 4)
            ImGui::SameLine();
    }

    // Zoom in
    {
        std::string label = "Zoom In";
        if (ImGui::Button(label.c_str()))
            setup_play_path(numPOI - 3);
        ImGui::SameLine();
    }

    // Zoom out
    {
        std::string label = "Zoom Out";
        if (ImGui::Button(label.c_str()))
            setup_play_path(numPOI - 2);
        ImGui::SameLine();
    }

    // Close up
    {
        std::string label = "Close up";
        if (ImGui::Button(label.c_str()))
            setup_play_path(numPOI - 1);
    }
}

void CameraController::setup_play_path(uint32_t targetPoint)
{
    // Mark that we are playing
    m_IsPlaying = true;
    m_TargetPOI = targetPoint;

    // Reset the current play time
    m_PlayTime = 0.0;

    // Position
    m_PositionSpline[0] = m_Camera.position;
    m_PositionSpline[1] = m_POIArray[targetPoint].position;

    // Rotation
    m_RotationSpline[0] = matrix_to_quaternion(m_Camera.view);
    m_RotationSpline[1] = m_POIArray[targetPoint].rotation;

    // Make sure we are following the shortest path
    float l0 = length(m_RotationSpline[1] - m_RotationSpline[0]);
    float l1 = length(m_RotationSpline[1] + m_RotationSpline[0]);
    if (l1 < l0)
        m_RotationSpline[1] = negate(m_RotationSpline[1]);

    // Rotation
    m_FOVSpline[0] = m_Camera.fov;
    m_FOVSpline[1] = m_POIArray[targetPoint].fov;
}


void CameraController::process_key_event(uint32_t keyCode, bool state)
{
    switch (keyCode)
    {
        case 0x44: // D
            m_ControllerStates[(uint32_t)NavigationButtons::Right] = state;
            break;
        case 0x51: // A
            m_ControllerStates[(uint32_t)NavigationButtons::Left] = state;
            break;
        case 0x5A: // Z
            m_ControllerStates[(uint32_t)NavigationButtons::Forward] = state;
            break;
        case 0x53: // S
            m_ControllerStates[(uint32_t)NavigationButtons::Backward] = state;
            break;
        case 0x41: // A
            m_ControllerStates[(uint32_t)NavigationButtons::Up] = state;
            break;
        case 0x45: // E
            m_ControllerStates[(uint32_t)NavigationButtons::Down] = state;
            break;
        case 0xA0: // Shift
            m_ControllerStates[(uint32_t)NavigationButtons::Shift] = state;
            break;
    }
}

bool CameraController:: process_mouse_button(MouseButton button, bool)
{
    if (button == MouseButton::Right)
    {
        graphics::window::set_cursor_visibility(m_Window, m_ActiveInteraction);
        m_ActiveInteraction = !m_ActiveInteraction;
        return true;
    }
    return false;
}

bool CameraController::process_mouse_movement(int2 mouse, uint2 windowCenter, float4 screenSize)
{
    if (m_ActiveInteraction)
    {
        int2 mouv = int2({ mouse.x - (int)windowCenter.x, mouse.y - (int)windowCenter.y });
        m_Camera.angles.x -= mouv.x / screenSize.x * 5.0f;
        m_Camera.angles.y -= mouv.y / screenSize.y * 5.0f;
        return true;
    }
    return false;
}

void CameraController::process_mouse_wheel(int wheel)
{
    if (m_ActiveInteraction)
    {
        if (wheel > 0)
            m_Speed *= 2.0;
        else
            m_Speed /= 2.0;
    }
}

void CameraController::update(double deltaTime)
{
    if (!m_IsPlaying)
    {
        if (m_ActiveInteraction)
        {
            float3 forwardDir = float3({ m_Camera.view.m[2], m_Camera.view.m[6], m_Camera.view.m[10] });
            float3 rightDir = float3({ m_Camera.view.m[0], m_Camera.view.m[4], m_Camera.view.m[8] });
            float speed = m_Speed * (float)deltaTime;

            float3 displacement = float3({ 0.0, 0.0, 0.0 });
            if (m_ControllerStates[(uint32_t)NavigationButtons::Forward])
            {
                displacement.x += forwardDir.x * speed;
                displacement.y += forwardDir.y * speed;
                displacement.z += forwardDir.z * speed;
            }

            if (m_ControllerStates[(uint32_t)NavigationButtons::Backward])
            {
                displacement.x -= forwardDir.x * speed;
                displacement.y -= forwardDir.y * speed;
                displacement.z -= forwardDir.z * speed;
            }

            if (m_ControllerStates[(uint32_t)NavigationButtons::Left])
            {
                displacement.x -= rightDir.x * speed;
                displacement.y -= rightDir.y * speed;
                displacement.z -= rightDir.z * speed;
            }

            if (m_ControllerStates[(uint32_t)NavigationButtons::Right])
            {
                displacement.x += rightDir.x * speed;
                displacement.y += rightDir.y * speed;
                displacement.z += rightDir.z * speed;
            }

            m_Camera.position = m_Camera.position + displacement;
        }
    }
    else
    {
        // Done with playing
        if (m_PlayTime >= m_Duration)
        {
            m_IsPlaying = false;
            m_Camera.angles = m_POIArray[m_TargetPOI].angles;
        }
        else
        {
            float t = m_PlayTime / m_Duration;
            float3 pos = lerp(m_PositionSpline[0], m_PositionSpline[1], t);
            float4 rot = slerp(m_RotationSpline[0], m_RotationSpline[1], t);
            rot = normalize(rot);
            float fov = lerp(m_FOVSpline[0], m_FOVSpline[1], t);

            // Update the camera data
            m_Camera.position = pos;
            m_Camera.view = quaternion_to_matrix(rot);
            m_Camera.fov = fov;

            // Add the time
            m_PlayTime += (float)deltaTime;
        }
    }

    // Position has been updated, update the matrices
    evaluate_camera_matrices();
}

void CameraController::evaluate_camera_matrices()
{
    // Evaluate the projection matrix
    m_Camera.projection = projection_matrix(m_Camera.fov, m_Camera.nearFar.x, m_Camera.nearFar.y, m_Camera.aspectRatio);

    if (!m_IsPlaying)
    {
        // Update the view matrix
        const float4x4 rotation_z = rotation_matrix_z(m_Camera.angles.z);
        const float4x4 rotation_y = rotation_matrix_y(m_Camera.angles.x);
        const float4x4 rotation_x = rotation_matrix_x(m_Camera.angles.y);
        m_Camera.view = mul(rotation_z, mul(rotation_x, rotation_y));
    }

    // Update the compound matrices
    m_Camera.viewProjection = mul(m_Camera.projection, m_Camera.view);

    // Compute the inverse matrices
    m_Camera.invViewProjection = inverse(m_Camera.viewProjection);
}

void CameraController::save_camera_path(const char* pathName)
{
    std::ofstream pathFile;
    pathFile.open(m_PathsDir + pathName);
    pathFile << m_POIArray.size() << std::endl;
    for (uint32_t ptIdx = 0; ptIdx < m_POIArray.size(); ++ptIdx)
    {
        const Transform& cT = m_POIArray[ptIdx];
        pathFile << cT.rotation.x << ";" << cT.rotation.y << ";" << cT.rotation.z << ";" << cT.rotation.w
            << ";" << cT.position.x << ";" << cT.position.y << ";" << cT.position.z
            << ";" << cT.angles.x << ";" << cT.angles.y << ";" << cT.angles.z
            << ";" << cT.fov << ";" << std::endl;
    }
    pathFile.close();
}

void CameraController::load_camera_path(const char* pathName)
{
    // Open the file
    std::ifstream pathFile;
    pathFile.open(m_PathsDir + pathName);
    if (!pathFile.is_open())
        return;

    // Read the number of control points
    uint32_t numPoints;
    pathFile >> numPoints;
    m_POIArray.resize(numPoints);

    // Read the individual points
    std::string line;
    for (uint32_t ptIdx = 0; ptIdx < numPoints; ++ptIdx)
    {
        // Read the line
        pathFile >> line;

        // Decompose it
        std::istringstream iss(line);
        Transform& cT = m_POIArray[ptIdx];
        char s;
        iss >> cT.rotation.x >> s >> cT.rotation.y >> s >> cT.rotation.z >> s >> cT.rotation.w
            >> s >> cT.position.x >> s >> cT.position.y >> s >> cT.position.z
            >> s >> cT.angles.x >> s >> cT.angles.y >> s >> cT.angles.z
            >> s >> cT.fov;
    }
    pathFile.close();
}

void CameraController::move_to_poi(uint32_t poiIDX)
{
    assert_msg(poiIDX < m_POIArray.size(), "POI not defined.");
    m_Camera.position = m_POIArray[poiIDX].position;
    m_Camera.angles = m_POIArray[poiIDX].angles;
    m_Camera.fov = m_POIArray[poiIDX].fov;
}