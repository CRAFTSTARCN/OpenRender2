# Open Render 2

[English](README.md) | [中文](README.zh-CN.md)

Open Render 2 是一个基于 C++17 的实时渲染器。它面向 Vulkan，采用基于 Mesh Shader 的延迟着色管线。

## 技术栈

- **图形 API：** Vulkan 1.x，使用 `VK_EXT_mesh_shader`
- **着色语言：** GLSL → SPIR-V 1.5
- **数学库：** GLM
- **窗口 / 输入：** GLFW
- **资产导入：** Assimp
- **JSON：** json11
- **日志：** spdlog
- **GPU 内存：** VulkanMemoryAllocator（VMA）
- **图像加载：** stb

## 目录结构

| 目录 | 职责 |
|---|---|
| `OpenRender2/` | 可执行层及高层引擎循环、关卡、对象系统 |
| `OpenRenderRuntime/` | 核心渲染静态库：RHI、渲染通道、场景、资源、资产系统 |
| `MeshletBuilder/` | 离线工具，将 FBX/OBJ 几何体烘焙为 Meshlet `.mlt` 文件 |
| `PyShaderBulider/` | 离线 Python 管线，编译固定渲染通道着色器并生成逐材质着色器 |
| `PassShader/` | 固定通道 GLSL 源码、共享头文件、模板及编译后的 SPIR-V 输出 |
| `Content/` | 运行时资产：关卡、材质、网格、纹理 |
| `ThirdParty/` | 嵌入式依赖：assimp、glfw、glm、json11、spdlog、stb、VMA |
| `Build/` | CMake 生成的构建树（git 忽略） |
| `Binary/` | 构建后输出目录，包含可执行文件、DLL、内容与着色器（git 忽略） |

## 架构分层

### OpenRender2 — 可执行层

- `OpenRender2.cpp`：入口点。加载 `Project.json` 与 `Config.ini`，构造 `OpenRenderEngine`，并运行 `PreInit → Init → EngineLoop → Terminate`。
- `Engine/Core/OpenRenderEngine.h`：单例 `GEngine`，持有 `Render`、`AssetSystem`、`InputSystem`、`ViewportClient` 与 `RenderSwapDataCenter`。
- `Engine/Core/LLevel.h`：关卡容器，包含 `GObject`、相机、方向光及天空盒 / IBL 纹理。
- `Engine/Core/GObject.h` / `ObjectScript.h`：实体 / 组件行为模型。
- `Engine/CoreObjects/StaticMeshComponent.h`：组件，负责导入网格 / 材质，并向渲染线程推送可渲染实例。

### OpenRenderRuntime — 渲染库

- `Core/Render/Render.h`：顶层 `Render` 类，持有 `RenderScene`、`RenderResource`、`RenderBlackboard`、`RHI` 及内置渲染通道。
- `Core/RHI/RHI.h`：抽象渲染硬件接口。
- `Core/VulkanRHI/`：基于 VMA 与命令池的具体 Vulkan 后端。
- `Core/RenderScene/RenderScene.h`：场景表达，包含相机、光源、可渲染实例及带视锥剔除的渲染队列。
- `Core/RenderResource/RenderResource.h`：GPU 端缓存，管理网格、材质、纹理、GBuffer、深度缓冲及全局 UBO。
- `Core/RenderPass/`：抽象通道框架及内置 `MeshDrawPass` 与 `DeferredShadingPass`。
- `Modules/AssetSystem/`：资产注册表与关卡、网格、材质、纹理导入器。
- `Modules/RenderData/`：`RenderSwapDataCenter` 与用于逻辑线程到渲染线程通信的类型化解析器。

### 工具

- `MeshletBuilder/`：离线烘焙器。通过 Assimp 读取 FBX，将三角形聚类为 Meshlet，计算包围体 / 法线锥，并写入 `.mlt` 二进制文件。
- `PyShaderBulider/`：离线编译器。`Compiler.py` 编译固定通道 SPIR-V；`ShaderCompiler.py` 解析 `.shader` 材质标记，输出逐材质 SPIR-V 与 `M_*.json` 描述文件。

### 内容资产

- `Content/Levels/L_*.json`：关卡描述。
- `Content/Materials/M_*.json`：材质基础；`MI_*.json`：实例；`Shaders/*.shader`：材质源码。
- `Content/Mesh/S_*.json`：静态网格包，引用源 FBX 与烘焙后的 `.mlt`。
- `Content/Texture/T_*.json`：纹理描述。

## 渲染管线

渲染器目前采用双通道延迟着色管线：

1. **MeshDrawPass**：Task / Mesh / Fragment 着色器将几何数据写入多目标 GBuffer（`GBufferA`–`GBufferE`）及深度缓冲。
2. **DeferredShadingPass**：全屏通道读取 GBuffer 并计算光照，最后写入交换链。

资源绑定按描述符集划分：

- **Set 0：** 全局 Uniform Buffer（相机、光源、环境光、帧数据）
- **Set 1：** 网格 Storage Buffer（顶点、Meshlet、顶点索引、图元索引）
- **Set 2：** 材质 Uniform Buffer 与采样纹理
- **Set 3：** 动态绘制调用 / 实例数据

## 线程与数据流

引擎通过 `Barrier(2)` 同步两条主线程：

- **逻辑线程**：处理输入、活跃关卡及对象脚本；以类型化 `RenderSwapData` 消息向渲染线程推送变更。
- **渲染线程**：消费 Swap Data，准备场景与资源，执行剔除、队列生成，并录制 `MeshDrawPass` / `DeferredShadingPass` 命令缓冲。

通信通过 `RenderSwapDataCenter` 完成，它维护前后双缓冲 Swap Data 向量，并在每帧边界通过 `EndLoopSwap()` 交换。

## 资产与 Shader 管线

### Meshlet

1. `MeshletBuilder` 通过 Assimp 导入 FBX。
2. 三角形被聚类为 Meshlet（最多 64 个顶点、126 个图元）。
3. 计算每个 Meshlet 的 AABB、包围球及法线锥。
4. 输出序列化为 `Content/Mesh/<name>/<name>.mlt`。

运行时，`MeshPackageImporter` 加载 FBX + `.mlt`，注册 `MeshPackageAsset`，并向渲染线程入队 `MeshCreateData`。

### 着色器与材质

1. 固定通道源码位于 `PassShader/src/`，共享头文件位于 `PassShader/include/`。
2. `PyShaderBulider/Compiler.py` 调用 `glslc` 生成 `PassShader/spv/*.spv`。
3. 材质作者在 `Content/Materials/Shaders/*.shader` 中使用简单标记语言编写材质。
4. `PyShaderBulider/ShaderCompiler.py` 生成中间 GLSL、编译逐材质 SPIR-V，并写入 `Content/Materials/M_*.json`。
5. 运行时，`MaterialBaseImporter` 加载 JSON + SPIR-V 字节，并向渲染线程发送 `MaterialBaseCreateData`。

> **注意：** 着色器编译目前仍是手动离线步骤；CMake 不会自动调用 Python 编译器。

## 构建

依赖：
- CMake 3.19+
- C++17 编译器
- 包含 `vulkan-1.lib` 与 `glslc` 的 Vulkan SDK
- Python 3.9+（用于着色器 / 材质编译）

步骤：

```bash
# 1. 配置
cmake -S . -B Build

# 2. 构建
cmake --build Build --config Release

# 3. 编译固定着色器（手动步骤）
cd PyShaderBulider
python Compiler.py
```

CMake 构建后命令会将 `Content/`、`PassShader/`、`Config.ini`、`Project.json` 复制到 `Binary/`。可执行文件从 `Binary/` 运行。

## 依赖

- [assimp](https://github.com/assimp/assimp)
- [glfw](https://github.com/glfw/glfw)
- [glm](https://github.com/g-truc/glm)
- [json11](https://github.com/dropbox/json11)
- [spdlog](https://github.com/gabime/spdlog)
- [stb](https://github.com/nothings/stb)
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
