#include "OpenRenderRuntime/Modules/RenderData/Resolvers/SceneDataResolvers.h"

#include "OpenRenderRuntime/Core/RenderScene/RenderScene.h"
#include "OpenRenderRuntime/Modules/RenderData/SceneSwapData.h"
#include "OpenRenderRuntime/Util/Logger.h"
#include "OpenRenderRuntime/Core/Render/Render.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/gtc/matrix_transform.hpp"
#include "glm/ext.hpp"

CameraSetResolver::CameraSetResolver(const RenderComponentsData& InData) : RenderSwapDataResolver(InData)
{
}

void CameraSetResolver::ResolveData(RenderSwapData* Data)
{
	DYNAMIC_CAST_DATA_CHECK(CameraSetData, Data, SetData)
	ScenePtr->Camera.SetupCamera(
		SetData->Position,
		SetData->PitchYawRoll,
		SetData->Near, SetData->Far,
		SetData->FOV, SetData->Aspect);

	delete SetData;
}

CameraPositionLookResolver::CameraPositionLookResolver(const RenderComponentsData& InData): RenderSwapDataResolver(InData)
{
}

void CameraPositionLookResolver::ResolveData(RenderSwapData* Data)
{
	DYNAMIC_CAST_DATA_CHECK(CameraPositionLookData, Data, SetData)
	ScenePtr->Camera.UpdateViewPositionLook(SetData->NewPosition, SetData->NewRotation);
	delete SetData;
}

DirectionalLightResolver::DirectionalLightResolver(const RenderComponentsData& InData) : RenderSwapDataResolver(InData)
{
}

void DirectionalLightResolver::ResolveData(RenderSwapData* Data)
{
	DYNAMIC_CAST_DATA_CHECK(DirectionalLightData, Data, LightData)
	DirectionalLight& Light = ScenePtr->Light;
	Light.Color = LightData->Color;
	if(LightData->UseAbsDir)
	{
		Light.DirectionL = LightData->DirectionL;
	}
	else
	{
		Light.DirectionL = glm::rotate(LightData->Rotation,glm::vec3(0.0f, 1.0f, 0.0f));
	}

	delete LightData;
}

EnvironmentDataResolver::EnvironmentDataResolver(const RenderComponentsData& InData): RenderSwapDataResolver(InData)
{
}

void EnvironmentDataResolver::ResolveData(RenderSwapData* Data)
{
	DYNAMIC_CAST_DATA_CHECK(EnvironmentTextureData, Data, EnvData)
	auto FindAndSetTex = [this](size_t Id, RHITexture*& Texture)
	{
		if(auto Iter = ResourcePtr->Textures.find(Id); Iter == ResourcePtr->Textures.end())
		{
			LOG_WARN_FUNCTION("No texture id {}, setted for environment mapping data");
		}
		else
		{
			Texture = Iter->second;
		}
	};
	IBLResource& IBL = ResourcePtr->GlobalIBLResource;
	FindAndSetTex(EnvData->SkyboxId, IBL.SkyBox);
	FindAndSetTex(EnvData->RadianceMapId, IBL.RadianceMap);
	FindAndSetTex(EnvData->IrradianceMapId, IBL.IrradianceMap);
	FindAndSetTex(EnvData->BRDFLUTId, IBL.BRDFLUT);

	RenderPtr->OnUpdateGlobalIBLResource(IBL);
	
	delete Data;
}

