# Intel® Texture Set Neural Compression

## Sample Overview
This repository contains the source code for a sample application that demonstrates the use of [Cooperative Vectors](https://devblogs.microsoft.com/directx/enabling-neural-rendering-in-directx-cooperative-vector-support-coming-soon/), a new feature introduced in DirectX 12. This feature enables standardized, cross-vendor access to hardware-accelerated matrix multiplication units.

The demo was presented at the Game Developers Conference (GDC) 2025, as part of the [Advanced Graphics Summit sessions](https://schedule.gdconf.com/session/advanced-graphics-summit-cooperative-vectors-and-neural-rendering/911753).

You can find some additional informations [here](https://community.intel.com/t5/Blogs/Tech-Innovation/Artificial-Intelligence-AI/Intel-Co-Presents-Cooperative-Vectors-with-Microsoft-at-Game/post/1674845).

## Getting Started

### Prerequisites
* A GPU that supports DirectX 12 Cooperative Vectors, such as:
  * Intel® Graphics LunarLake (or newer)
  * Intel® Arc B-Series Graphics Cards (or newer)
* A [driver](https://www.intel.com/content/www/us/en/download/785597) that supports DirectX 12 Cooperative Vectors (or newer)
* Enable [Windows Developer Mode](https://learn.microsoft.com/en-us/windows/apps/get-started/enable-your-device-for-development)
* Compiler with C++20 support
  * [Microsoft Visual Studio 2022](https://www.visualstudio.com/) or newer
* [CMake 3.8](https://cmake.org/) or newer

### Clone repository

    git clone https://github.com/GameTechDev/TextureSetNeuralCompressionSample.git
    cd TextureSetNeuralCompressionSample

### Install dependencies

Download the Microsoft® DirectX 12 Agility SDK version 717 (preview or later) and the Microsoft® DirectX Compiler version 8.2502.8 (or later). You can do this manually or run the **dependencies.bat** script to automatically download and install the required dependencies.

### Build

    mkdir build
    cd build
    cmake .. -DDX12_SDK_VERSION=717
    cmake --build . --config Release

### Run the sample

The sample executable is called `dino_danger`. You can run it from the **Microsoft Visual Studio 2022 IDE** or via the command line:

    cd output/bin/Release
    dino_danger.exe --data-dir ../../..

You can also get more info about the command line options by running:

    dino_danger.exe --help
        --data-dir Location of the resource folders.
        --adapter-id Integer that allows to pick the desired GPU [-1 = Largest VRAM, >= 0 System adapter ID].
        --poi Integer that allows to pick the initial camera location.
        --disable-coop Disable cooperative vector usage at launch.
        --disable-animation Disable mesh animation at launch.
        --rendering-mode Pick the rendering mode [0 = Material, 1 = GBuffer, 2 = Debug].
        --texture-mode Pick the texture mode [0 = Uncompressed, 1 = BC6, 2 = Neural].
        --filtering-mode Pick the filtering mode [0 = Nearest, 1 = Linear, 2 = Anisotropic].

## License
Intel® Texture Set Neural Compression Sample is licensed under the [MIT License](LICENSE).