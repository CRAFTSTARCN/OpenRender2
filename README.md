# Open Render 2

[English](README.md) | [中文](README.zh-CN.md)

Open Render 2 is a real-time renderer built in C++17. It targets Vulkan with a mesh-shader-based deferred shading pipeline.

## Technology Stack

- **Graphics API:** Vulkan 1.x with `VK_EXT_mesh_shader`
- **Shading Language:** GLSL → SPIR-V 1.5
- **Math:** GLM
- **Windowing / Input:** GLFW
- **Asset Import:** Assimp
- **JSON:** json11
- **Logging:** spdlog
- **GPU Memory:** VulkanMemoryAllocator (VMA)
- **Image Loading:** stb

## Directory Structure

| Directory | Role |
|---|---|
| `OpenRender2/` | Executable and high-level engine loop / level / object system |
| `OpenRenderRuntime/` | Core renderer static library: RHI, render passes, scene, resources, asset system |
| `MeshletBuilder/` | Offline tool that bakes FBX/OBJ geometry into meshlet `.mlt` files |
| `PyShaderBulider/` | Offline Python pipeline that compiles fixed render-pass shaders and generates per-material shaders |
| `PassShader/` | Fixed-pass GLSL sources, shared includes, templates, and compiled SPIR-V outputs |
| `Content/` | Runtime assets: levels, materials, meshes, and textures |
| `ThirdParty/` | Embedded dependencies: assimp, glfw, glm, json11, spdlog, stb, VMA |
| `Build/` | CMake-generated build tree (ignored by git) |
| `Binary/` | Post-build output directory with executables, DLLs, content, and shaders (ignored by git) |

## Architecture Layers

### OpenRender2 — Executable Layer

- `OpenRender2.cpp`: entry point. Loads `Project.json` and `Config.ini`, constructs `OpenRenderEngine`, and runs `PreInit → Init → EngineLoop → Terminate`.
- `Engine/Core/OpenRenderEngine.h`: singleton `GEngine` that owns `Render`, `AssetSystem`, `InputSystem`, `ViewportClient`, and `RenderSwapDataCenter`.
- `Engine/Core/LLevel.h`: level container with `GObject`s, camera, directional light, and skybox/IBL textures.
- `Engine/Core/GObject.h` / `ObjectScript.h`: entity/component behavior model.
- `Engine/CoreObjects/StaticMeshComponent.h`: component that imports a mesh/material and pushes renderable instances to the render thread.

### OpenRenderRuntime — Renderer Library

- `Core/Render/Render.h`: top-level `Render` class. Owns `RenderScene`, `RenderResource`, `RenderBlackboard`, `RHI`, and the built-in passes.
- `Core/RHI/RHI.h`: abstract rendering hardware interface.
- `Core/VulkanRHI/`: concrete Vulkan backend using VMA and command pools.
- `Core/RenderScene/RenderScene.h`: scene representation, camera, lights, renderable instances, and render queues with frustum culling.
- `Core/RenderResource/RenderResource.h`: GPU-side cache for meshes, materials, textures, GBuffer, depth, and global UBO.
- `Core/RenderPass/`: abstract pass framework plus built-in `MeshDrawPass` and `DeferredShadingPass`.
- `Modules/AssetSystem/`: asset registry and importers for levels, meshes, materials, and textures.
- `Modules/RenderData/`: `RenderSwapDataCenter` and typed resolvers for logic→render thread communication.

### Tools

- `MeshletBuilder/`: offline baker. Reads FBX via Assimp, clusters triangles into meshlets, computes bounds/normal cones, and writes `.mlt` binaries.
- `PyShaderBulider/`: offline compiler. `Compiler.py` builds fixed pass SPIR-V; `ShaderCompiler.py` parses `.shader` material markup and emits per-material SPIR-V and `M_*.json` descriptors.

### Content

- `Content/Levels/L_*.json`: level descriptors.
- `Content/Materials/M_*.json`: material bases; `MI_*.json`: instances; `Shaders/*.shader`: material source.
- `Content/Mesh/S_*.json`: static mesh packs referencing source FBX and baked `.mlt`.
- `Content/Texture/T_*.json`: texture descriptors.

## Rendering Pipeline

The renderer currently uses a two-pass deferred shading pipeline:

1. **MeshDrawPass**: task/mesh/fragment shaders write geometry data into a multi-target GBuffer (`GBufferA`–`GBufferE`) plus depth.
2. **DeferredShadingPass**: a full-screen pass reads the GBuffer and computes lighting before writing to the swapchain.

Resource binding is split across descriptor sets:

- **Set 0:** Global uniform buffer (camera, light, ambient, frame data)
- **Set 1:** Mesh storage buffers (vertices, meshlets, vertex indices, primitive indices)
- **Set 2:** Material uniform buffer and sampled textures
- **Set 3:** Dynamic draw-call / instance data

## Threading & Data Flow

The engine runs two main threads synchronized by a `Barrier(2)`:

- **Logic Thread**: ticks input, the active level, and object scripts; pushes changes to the render thread as typed `RenderSwapData` messages.
- **Render Thread**: consumes swap data, prepares the scene and resources, culls, forms queues, and records `MeshDrawPass` / `DeferredShadingPass` command buffers.

Communication uses `RenderSwapDataCenter`, which maintains front/back swap-data vectors and exchanges them at frame boundaries with `EndLoopSwap()`.

## Asset & Shader Pipeline

### Meshlets

1. `MeshletBuilder` imports an FBX with Assimp.
2. Triangles are clustered into meshlets (max 64 vertices, 126 primitives).
3. Per-meshlet AABB, bounding sphere, and normal cone are computed.
4. Output is serialized to `Content/Mesh/<name>/<name>.mlt`.

At runtime, `MeshPackageImporter` loads the FBX + `.mlt`, registers a `MeshPackageAsset`, and enqueues `MeshCreateData` for the render thread.

### Shaders & Materials

1. Fixed pass sources live in `PassShader/src/` and share headers in `PassShader/include/`.
2. `PyShaderBulider/Compiler.py` invokes `glslc` to produce `PassShader/spv/*.spv`.
3. Material authors write `Content/Materials/Shaders/*.shader` using a simple markup language.
4. `PyShaderBulider/ShaderCompiler.py` generates intermediate GLSL, compiles per-material SPIR-V, and writes `Content/Materials/M_*.json`.
5. At runtime, `MaterialBaseImporter` loads the JSON + SPIR-V bytes and sends `MaterialBaseCreateData` to the render thread.

> **Note:** Shader compilation is currently a manual offline step; CMake does not invoke the Python compiler automatically.

## Build

Requirements:
- CMake 3.19+
- C++17 compiler
- Vulkan SDK with `vulkan-1.lib` and `glslc`
- Python 3.9+ (for shader/material compilation)

Steps:

```bash
# 1. Configure
cmake -S . -B Build

# 2. Build
cmake --build Build --config Release

# 3. Compile fixed shaders (manual step)
cd PyShaderBulider
python Compiler.py
```

CMake post-build commands copy `Content/`, `PassShader/`, `Config.ini`, and `Project.json` into `Binary/`. The executable runs from `Binary/`.

## Dependencies

- [assimp](https://github.com/assimp/assimp)
- [glfw](https://github.com/glfw/glfw)
- [glm](https://github.com/g-truc/glm)
- [json11](https://github.com/dropbox/json11)
- [spdlog](https://github.com/gabime/spdlog)
- [stb](https://github.com/nothings/stb)
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
