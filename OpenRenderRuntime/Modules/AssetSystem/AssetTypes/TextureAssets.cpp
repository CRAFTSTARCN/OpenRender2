#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/TextureAssets.h"

TextureAsset::TextureAsset(size_t InTextureId) : TextureId(InTextureId)
{
}

TextureAsset::~TextureAsset()
{
}

size_t TextureAsset::GetTexId() const
{
	return TextureId;
}

Texture2DAsset::Texture2DAsset(size_t InTextureId) : TextureAsset(InTextureId)
{
}

Texture2DAsset::~Texture2DAsset()
{
}

TextureCubeAsset::TextureCubeAsset(size_t InTextureId) : TextureAsset(InTextureId)
{
}

TextureCubeAsset::~TextureCubeAsset()
{
}
