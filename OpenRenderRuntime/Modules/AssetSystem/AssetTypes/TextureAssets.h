#pragma once
#include "OpenRenderRuntime/Modules/AssetSystem/AssetObject.h"

class TextureAsset : public AssetObject
{
protected:

	size_t TextureId;
	
public:

	TextureAsset(size_t InTextureId);
	virtual ~TextureAsset() override;

	size_t GetTexId() const;
	
};

class Texture2DAsset final : public TextureAsset
{
	
public:

	Texture2DAsset(size_t InTextureId);
	~Texture2DAsset() override;

	DEFINE_ASSET_TYPE("Texture2D")
	
};

class TextureCubeAsset final : public TextureAsset
{
public:

	TextureCubeAsset(size_t InTextureId);
	~TextureCubeAsset() override;

	DEFINE_ASSET_TYPE("TextureCube")
};