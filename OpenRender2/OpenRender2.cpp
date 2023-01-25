#include "OpenRenderRuntime/Util/Logger.h"

#include <filesystem>

#include "OpenRenderRuntime/Core/Basic/RenderMath.h"
#include "OpenRenderRuntime/Core/Render/Render.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetSystem.h"
#include "OpenRenderRuntime/Modules/InputSystem/InputSystem.h"
#include "OpenRenderRuntime/Modules/RenderConfig/RenderCofig.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderableInstanceData.h"
#include "OpenRenderRuntime/Modules/RenderData/SceneSwapData.h"


int main(int argc, char* argv[])
{
    /*
     * Path enviroment setting
     */
    std::filesystem::path ExeDir = std::filesystem::path(argv[0]).parent_path();
    LOG_INFO_FUNCTION("Open Render 2 Running");
    LOG_INFO_FUNCTION("Base dir: {}", ExeDir.generic_string());

    RenderConfig::Get().RootDir = ExeDir.generic_string();

    /*
     * Subsystem init
     */
    Render Render;
    InputSystem Input;
    RenderSwapDataCenter SwapDataCenter;
    
    std::filesystem::path ContentDir = ExeDir / "Content";
    AssetSystem AssetSys(AssetSystemCreateParam{
        ContentDir.generic_string(),
        Render.GetRenderRHIShaderPlatform(),
        false});
    
    Render.PreInit();

    RenderWindowProxy& Window = Render.GetWindow();
    WindowSize CurrentSize =  Window.GetWindowSize();

    Render.Init(&SwapDataCenter);
    Input.Init(&Window);
    
    AssetSys.Init(&SwapDataCenter);

    /*
     * Asset loading
     */
    size_t SkyBoxAssetId =  AssetSys.Import("./Texture/AnimSky/T_Anim_Skybox.json");
    size_t BRDFLUTAssetId = AssetSys.Import("./Texture/T_BRDF_LUT.json");
    size_t Radiance = AssetSys.Import("./Texture/AnimSky/T_Anim_Radiance.json");
    size_t Irradiance = AssetSys.Import("./Texture/AnimSky/T_Anim_Irradiance.json");
    size_t SwordMeshAssetId = AssetSys.Import("./Mesh/S_Sword.json");
    size_t BunnyMeshAssetId = AssetSys.Import("./Mesh/S_Bunny.json");
    size_t SphereAssetId = AssetSys.Import("./Mesh/S_Sphere.json");

    /*
     * Initial data
     */
    EnvironmentTextureData* EnvData = new EnvironmentTextureData;
    EnvData->BRDFLUTId = 1;
    EnvData->SkyboxId = 0;
    EnvData->RadianceMapId = 2;
    EnvData->IrradianceMapId = 3;
    
    CameraSetData* CameraData = new CameraSetData;
    CameraData->Position = glm::vec3{0,0,0};
    CameraData->PitchYawRoll = glm::vec3{0, -90.0f, 0};
    CameraData->Aspect = (float)CurrentSize.Width / (float)CurrentSize.Height;
    CameraData->Near = 0.1f;
    CameraData->Far = 10000;
    CameraData->FOV = 75.0f;
    SwapDataCenter.GetLogicSide().push_back(EnvData);
    SwapDataCenter.GetLogicSide().push_back(CameraData);

    Transform BunnyTrans {glm::vec3(0, -10, -50), glm::quat(glm::radians(glm::vec3(-90.0f, -45.0f, 0.0f))), glm::vec3(20)};

    RenderableInstanceAdd* AddInstance0 = new RenderableInstanceAdd;
    AddInstance0->RenderableInstanceId = 0;
    AddInstance0->IsBlocker = true;
    AddInstance0->MeshId = 2;
    AddInstance0->MaterialInstanceId = 2;
    AddInstance0->ModelMatrix = ORMath::Transform2Matrix(BunnyTrans);

    RenderableInstanceAdd* AddInstance1 = new RenderableInstanceAdd;
    AddInstance1->RenderableInstanceId = 1;
    AddInstance1->IsBlocker = true;
    AddInstance1->MeshId = 3;
    AddInstance1->MaterialInstanceId = 2;
    AddInstance1->ModelMatrix = ORMath::Transform2Matrix(BunnyTrans);

    SwapDataCenter.GetLogicSide().push_back(AddInstance0);
    SwapDataCenter.GetLogicSide().push_back(AddInstance1);

    DirectionalLightData* LightData = new DirectionalLightData;
    LightData->Color = glm::vec3(0.7f, 0.8f, 1.0f);
    LightData->DirectionL = glm::normalize(glm::vec3(0.0f, 0.72f, 0.72f));
    LightData->UseAbsDir = true;
    SwapDataCenter.GetLogicSide().push_back(LightData);
    
    SwapDataCenter.EndLoopSwap();

    Render.InitialLoading();
    
    SwapDataCenter.EndLoopSwap();

    /*
     * Loop prepare
     */

    auto Now = std::chrono::steady_clock::now();
    
    glm::vec3 CurCameraPos = glm::vec3(0, 0, 0);
    glm::vec3 CurCameraRot = glm::vec3(0, -90, 0);
    float Speed = 70.0f;
    float LookSpeed = 0.1f;
    glm::vec3 Forward = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);
    
    while(!Window.WindowShouldClose())
    {
        auto Now2 = std::chrono::steady_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::duration<float>>(Now2 - Now);
        Now = Now2;
        Input.Tick(dur.count());
        

        bool Dirty = false;
        if(Input.GetMouseButtonDown(MouseButton_Right))
        {

            glm::vec2 MouseMove = Input.GetMouseMovement();
            if(std::abs(MouseMove.x) > 0.0f || std::abs(MouseMove.y) > 0.0f)
            {
                CurCameraRot.y += MouseMove.x * LookSpeed;
                if(CurCameraRot.y > 360.0f)
                {
                    CurCameraRot.y -= 360.0f;
                }
                CurCameraRot.x -= MouseMove.y * LookSpeed;

                if(CurCameraRot.x > 89.0f)
                {
                    CurCameraRot.x = 89.0f;
                }
                else if(CurCameraRot.x < -89.0f)
                {
                    CurCameraRot.x = -89.0f;
                }

                Forward.y = sin(glm::radians(CurCameraRot.x));
                Forward.x = cos(glm::radians(CurCameraRot.x)) * cos(glm::radians(CurCameraRot.y));
                Forward.z = cos(glm::radians(CurCameraRot.x)) * sin(glm::radians(CurCameraRot.y));
                Forward = glm::normalize(Forward);

                Right = glm::normalize(glm::cross(Forward, glm::vec3(0,1,0)));
                
                Dirty = true;
            }
            
            if(Input.GetKeyDown(InputKey_W))
            {
                CurCameraPos += Speed * dur.count() * Forward;
                Dirty = true;
            }

            if(Input.GetKeyDown(InputKey_S))
            {
                CurCameraPos -= Speed * dur.count() * Forward;
                Dirty = true;
            }

            if(Input.GetKeyDown(InputKey_D))
            {
                CurCameraPos += Speed * dur.count() * Right;
                Dirty = true;
            }

            if(Input.GetKeyDown(InputKey_A))
            {
                CurCameraPos -= Speed * dur.count() * Right;
                Dirty = true;
            }
            
        }
        
        if(Dirty)
        {
            CameraPositionLookData* CamLookPosData = new CameraPositionLookData;
            CamLookPosData->NewPosition = CurCameraPos;
            CamLookPosData->NewRotation = CurCameraRot;
            SwapDataCenter.GetLogicSide().push_back(CamLookPosData);
        }
        
        Render.Tick(dur.count());
        SwapDataCenter.EndLoopSwap();
    }
    Render.Terminate();
    
    return 0;
}
