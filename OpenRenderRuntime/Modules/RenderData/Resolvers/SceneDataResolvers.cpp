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
	auto FindAndSetTex = [this](size_t Id, RenderTexture*& NewTexture, RenderTexture*& StatTexture)
	{
		if(Id == RenderTexture::BAD_TEXTURE_ID)
		{
			NewTexture = nullptr;
			return;
		}
		if(auto Iter = ResourcePtr->Textures.find(Id); Iter == ResourcePtr->Textures.end())
		{
			LOG_WARN_FUNCTION("No texture id {}, setted for environment mapping data");
			NewTexture = nullptr;
		}
		else
		{
			NewTexture = Iter->second;
			StatTexture = NewTexture;
		}
	};

	IBLResource NewIBL;
	IBLResource& IBL = ResourcePtr->GlobalIBLResource;
	FindAndSetTex(EnvData->SkyboxId, NewIBL.SkyBox, IBL.SkyBox);
	FindAndSetTex(EnvData->RadianceMapId, NewIBL.RadianceMap, IBL.RadianceMap);
	FindAndSetTex(EnvData->IrradianceMapId, NewIBL.IrradianceMap, IBL.IrradianceMap);
	FindAndSetTex(EnvData->BRDFLUTId, NewIBL.BRDFLUT, IBL.BRDFLUT);

	RenderPtr->OnUpdateGlobalIBLResource(NewIBL);
	
	delete Data;
}

