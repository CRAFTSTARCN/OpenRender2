#include "OpenRender2/Engine/Core/LLevel.h"

#include "OpenRenderEngine.h"
#include "OpenRenderRuntime/Modules/RenderData/SceneSwapData.h"
#include "OpenRenderRuntime/Util/Logger.h"

void LevelCamera::SendPosRotData2Swap()
{
	CameraPositionLookData* NewCamPosLook = new CameraPositionLookData;
	NewCamPosLook->NewPosition = Data.Position;
	NewCamPosLook->NewRotation = Data.Rotation;
	
	GEngine->GetSwapDataCenter()->GetLogicSide().push_back(NewCamPosLook);
}

void LevelCamera::SenFullData2Swap()
{
	CameraSetData* NewCamSet = new CameraSetData;
	NewCamSet->Aspect = (float)CaptureWidth / (float)CaptureHeight;
	NewCamSet->FOV = Data.FOVV;
	NewCamSet->Near = Data.Near;
	NewCamSet->Far = Data.Far;
	NewCamSet->Position = Data.Position;
	NewCamSet->PitchYawRoll = Data.Rotation;

	GEngine->GetSwapDataCenter()->GetLogicSide().push_back(NewCamSet);
}

void LevelCamera::Serialize(CameraData&& InCamData)
{
	Data = std::move(InCamData);
	
	if(abs(Data.Rotation.x) > 89.0f)
	{
		Data.Rotation.x = Data.Rotation.x > 0 ? 89.0f : -89.0f;
		LOG_WARN_FUNCTION("Overflow look up and down lock, use 89.0f");
	}
	
	Forward.y = sin(glm::radians(Data.Rotation.x));
	Forward.x = cos(glm::radians(Data.Rotation.x)) * cos(glm::radians(Data.Rotation.y));
	Forward.z = cos(glm::radians(Data.Rotation.x)) * sin(glm::radians(Data.Rotation.y));
	Forward = glm::normalize(Forward);

	Right = glm::normalize(glm::cross(Forward, glm::vec3(0,1,0)));

	CaptureWidth = GEngine->GetClient()->GetRenderingWidth();
	CaptureHeight = GEngine->GetClient()->GetRenderingHeight();
}

void LevelCamera::Begin()
{
	SenFullData2Swap();
}

void LevelCamera::Tick(float DeltaTime)
{
	InputSystem* Input = GEngine->GetInputSystem();
	if(Input->GetMouseButtonDown(MouseButton_Right))
	{

		glm::vec2 MouseMove = Input->GetMouseMovement();
		if(std::abs(MouseMove.x) > 0.0f || std::abs(MouseMove.y) > 0.0f)
		{
			Data.Rotation.y += MouseMove.x * Data.CameraLookSpeed;
			if(Data.Rotation.y > 360.0f)
			{
				Data.Rotation.y -= 360.0f;
			}
			Data.Rotation.x -= MouseMove.y * Data.CameraLookSpeed;

			if(Data.Rotation.x > 89.0f)
			{
				Data.Rotation.x = 89.0f;
			}
			else if(Data.Rotation.x < -89.0f)
			{
				Data.Rotation.x = -89.0f;
			}

			Forward.y = sin(glm::radians(Data.Rotation.x));
			Forward.x = cos(glm::radians(Data.Rotation.x)) * cos(glm::radians(Data.Rotation.y));
			Forward.z = cos(glm::radians(Data.Rotation.x)) * sin(glm::radians(Data.Rotation.y));
			Forward = glm::normalize(Forward);

			Right = glm::normalize(glm::cross(Forward, glm::vec3(0,1,0)));
                
			DirtyLookPos = true;
		}
            
		if(Input->GetKeyDown(InputKey_W))
		{
			Data.Position += Data.CameraMoveSpeed * DeltaTime * Forward;
			DirtyLookPos = true;
		}

		if(Input->GetKeyDown(InputKey_S))
		{
			Data.Position -= Data.CameraMoveSpeed * DeltaTime * Forward;
			DirtyLookPos = true;
		}

		if(Input->GetKeyDown(InputKey_D))
		{
			Data.Position += Data.CameraMoveSpeed * DeltaTime * Right;
			DirtyLookPos = true;
		}

		if(Input->GetKeyDown(InputKey_A))
		{
			Data.Position -= Data.CameraMoveSpeed * DeltaTime * Right;
			DirtyLookPos = true;
		}
            
	}

	uint32_t NewWidth{GEngine->GetClient()->GetRenderingWidth()}, NewHeight{GEngine->GetClient()->GetRenderingHeight()};
	if(NewWidth != CaptureWidth || NewHeight != CaptureHeight)
	{
		CaptureWidth = NewWidth;
		CaptureHeight = NewHeight;
		DirtyAll = true;
	}

	if(DirtyAll)
	{
		SenFullData2Swap();
	}
	else if(DirtyLookPos)
	{
		SendPosRotData2Swap();
	}
}

LLevel::LLevel(size_t PreserveSpace)
{
	Objects.reserve(PreserveSpace);
}

void LLevel::Tick(float DeltaTime)
{
	LevelCamera.Tick(DeltaTime);
	
	size_t LastSize = Objects.size();
	size_t PreMoveSize = 0;
	
	for(size_t i=0 ; i<LastSize; ++i)
	{
		GObject* Object = Objects[i];
		if(Object->GetStatus() == GObjectStatus_PendingKill)
		{
			//Recycle body
			Object->LevelFinishDestroy();
			++PreMoveSize;
		}
		else if (Object->GetStatus() == GObjectStatus_Active)
		{
			if(PreMoveSize)
			{
				Objects[i - PreMoveSize] = Object;
			}
			Object->Tick(DeltaTime);
		}
		//Prepare object(in level, not finish spawn) will not tick
	}

	size_t TickPostSize = LastSize - PreMoveSize;

	size_t PostMoveSize = 0;
	for(size_t i=0; i < TickPostSize; ++i)
	{
		GObject* Object = Objects[i];
		if(Object->GetStatus() == GObjectStatus_PendingKill)
		{
			//Recycle body when post
			Object->LevelFinishDestroy();
			++PostMoveSize;
		}
		else if (Object->GetStatus() == GObjectStatus_Active)
		{
			if(PostMoveSize)
			{
				Objects[i - PostMoveSize] = Object;
			}
			Object->PostTick(DeltaTime);
		}
	}

	//Process new spawned objects while tick
	size_t NewSize = Objects.size();
	size_t MoveSize = PreMoveSize + PostMoveSize;
	if(MoveSize)
	{
		for(size_t i = LastSize; i < NewSize; ++i)
		{
			Objects[i - MoveSize] = Objects[i];
		}
	}

	Objects.resize(NewSize - MoveSize);
}

void LLevel::AddObject(GObject* Object)
{
	if(!Object)
	{
		LOG_ERROR_FUNCTION("Invalid object nullptr add to level");
	}

	Objects.push_back(Object);
}

void LLevel::SerializeFromAsset(LevelAsset* Asset)
{
	//Camera
	LevelCamera.Serialize(CameraData(Asset->InitCameraData));

	//Light
	LightData = Asset->LightData;

	//Skybox and IBL Lighting
	size_t SkyboxAssetId = GEngine->GetAssetSystem()->GetOrImport(Asset->SkyboxData.Skybox);
	size_t RadianceAssetId = GEngine->GetAssetSystem()->GetOrImport(Asset->SkyboxData.Radiance);
	size_t IrradianceAssetId = GEngine->GetAssetSystem()->GetOrImport(Asset->SkyboxData.Irradiance);

	auto GetAssetFromId = [this](size_t Id, TextureCubeAsset*& Asset, const std::string& Path)
	{
		if(Id == AssetRegistry::BAD_GASSET_ID)
		{
			LOG_ERROR_FUNCTION("Invalid skybox texture path {0}", Path);
		}
		else
		{
			AssetObject* LoadedAsset = GEngine->GetAssetSystem()->GetAssetById(Id);
			Asset = dynamic_cast<TextureCubeAsset*>(LoadedAsset);
			if(!Asset)
			{
				LOG_ERROR_FUNCTION("Asset type error, require texture cube asset, got {0}, path {1}", LoadedAsset->GetAssetTypeName(), Path);
			}
			else
			{
				Asset->IncreaseUsageCount();
			}
		}
	};

	GetAssetFromId(SkyboxAssetId, SkyboxAsset, Asset->SkyboxData.Skybox);
	GetAssetFromId(RadianceAssetId, RadianceAsset, Asset->SkyboxData.Radiance);
	GetAssetFromId(IrradianceAssetId, IrradianceAsset, Asset->SkyboxData.Irradiance);
	//Objects

	for(auto ObjectJson : Asset->ObjectJson)
	{
		GObject* NewObject = GObject::GetNewObject();
		NewObject->Serialize(ObjectJson);
		AddObject(NewObject);
	}
}

//Only happens while load level finish
void LLevel::Begin()
{
	//Setup camera
	LevelCamera.Begin();

	//Setup light
	DirectionalLightData* LightSwapData = new DirectionalLightData;
	LightSwapData->Color = glm::vec3(LightData.Color);
	LightSwapData->DirectionL = LightData.Direction;
	LightSwapData->UseAbsDir = true;
	GEngine->GetSwapDataCenter()->GetLogicSide().push_back(LightSwapData);
	

	//Setup skybox data
	EnvironmentTextureData* EnvData = new EnvironmentTextureData;
	EnvData->SkyboxId = SkyboxAsset ? SkyboxAsset->GetTexId() : RenderResource::BAD_TEXTURE_ID;
	EnvData->RadianceMapId = RadianceAsset ? RadianceAsset->GetTexId() : RenderResource::BAD_TEXTURE_ID;
	EnvData->IrradianceMapId = IrradianceAsset ? IrradianceAsset->GetTexId() : RenderResource::BAD_TEXTURE_ID;
	EnvData->BRDFLUTId = RenderResource::BAD_TEXTURE_ID;

	GEngine->GetSwapDataCenter()->GetLogicSide().push_back(EnvData);
	
	
	size_t CurrentSize = Objects.size();
	for(size_t i=0; i<CurrentSize; ++i)
	{
		Objects[i]->Activate();
	}
}

void LLevel::Unload()
{
	for(auto Obj : Objects)
	{
		Obj->Destroy();
		Obj->LevelFinishDestroy();
	}
}
