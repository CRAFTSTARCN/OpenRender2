#pragma once

#include <vulkan/vulkan.h>

struct VulkanGPUFeature
{
    size_t Offset = 0;
    size_t Weight = 1;
    bool ForceRequired = false;
};

inline constexpr VulkanGPUFeature Features[] = {
    {offsetof(VkPhysicalDeviceFeatures, robustBufferAccess)},
	{offsetof(VkPhysicalDeviceFeatures, fullDrawIndexUint32)},
	{offsetof(VkPhysicalDeviceFeatures, imageCubeArray)},
	{offsetof(VkPhysicalDeviceFeatures, independentBlend), 5, true},
	{offsetof(VkPhysicalDeviceFeatures, geometryShader), 5, true},
	{offsetof(VkPhysicalDeviceFeatures, tessellationShader)},
	{offsetof(VkPhysicalDeviceFeatures, sampleRateShading)},
	{offsetof(VkPhysicalDeviceFeatures, dualSrcBlend)},
	{offsetof(VkPhysicalDeviceFeatures, logicOp)},
	{offsetof(VkPhysicalDeviceFeatures, multiDrawIndirect)},
	{offsetof(VkPhysicalDeviceFeatures, drawIndirectFirstInstance)},
	{offsetof(VkPhysicalDeviceFeatures, depthClamp)},
	{offsetof(VkPhysicalDeviceFeatures, depthBiasClamp)},
	{offsetof(VkPhysicalDeviceFeatures, fillModeNonSolid), 1, true},
	{offsetof(VkPhysicalDeviceFeatures, depthBounds)},
	{offsetof(VkPhysicalDeviceFeatures, wideLines), 3, true},
	{offsetof(VkPhysicalDeviceFeatures, largePoints)},
	{offsetof(VkPhysicalDeviceFeatures, alphaToOne)},
	{offsetof(VkPhysicalDeviceFeatures, multiViewport)},
	{offsetof(VkPhysicalDeviceFeatures, samplerAnisotropy), 5, true},
	{offsetof(VkPhysicalDeviceFeatures, textureCompressionETC2)},
	{offsetof(VkPhysicalDeviceFeatures, textureCompressionASTC_LDR)},
	{offsetof(VkPhysicalDeviceFeatures, textureCompressionBC)},
	{offsetof(VkPhysicalDeviceFeatures, occlusionQueryPrecise)},
	{offsetof(VkPhysicalDeviceFeatures, pipelineStatisticsQuery)},
	{offsetof(VkPhysicalDeviceFeatures, vertexPipelineStoresAndAtomics)},
	{offsetof(VkPhysicalDeviceFeatures, fragmentStoresAndAtomics), 5, true},
	{offsetof(VkPhysicalDeviceFeatures, shaderTessellationAndGeometryPointSize)},
	{offsetof(VkPhysicalDeviceFeatures, shaderImageGatherExtended)},
	{offsetof(VkPhysicalDeviceFeatures, shaderStorageImageExtendedFormats)},
	{offsetof(VkPhysicalDeviceFeatures, shaderStorageImageMultisample)},
	{offsetof(VkPhysicalDeviceFeatures, shaderStorageImageReadWithoutFormat)},
	{offsetof(VkPhysicalDeviceFeatures, shaderStorageImageWriteWithoutFormat)},
	{offsetof(VkPhysicalDeviceFeatures, shaderUniformBufferArrayDynamicIndexing)},
	{offsetof(VkPhysicalDeviceFeatures, shaderSampledImageArrayDynamicIndexing)},
	{offsetof(VkPhysicalDeviceFeatures, shaderStorageBufferArrayDynamicIndexing)},
	{offsetof(VkPhysicalDeviceFeatures, shaderStorageImageArrayDynamicIndexing)},
	{offsetof(VkPhysicalDeviceFeatures, shaderClipDistance)},
	{offsetof(VkPhysicalDeviceFeatures, shaderCullDistance)},
	{offsetof(VkPhysicalDeviceFeatures, shaderFloat64)},
	{offsetof(VkPhysicalDeviceFeatures, shaderInt64)},
	{offsetof(VkPhysicalDeviceFeatures, shaderInt16)},
	{offsetof(VkPhysicalDeviceFeatures, shaderResourceResidency)},
	{offsetof(VkPhysicalDeviceFeatures, shaderResourceMinLod)},
	{offsetof(VkPhysicalDeviceFeatures, sparseBinding)},
	{offsetof(VkPhysicalDeviceFeatures, sparseResidencyBuffer)},
	{offsetof(VkPhysicalDeviceFeatures, sparseResidencyImage2D)},
	{offsetof(VkPhysicalDeviceFeatures, sparseResidencyImage3D)},
	{offsetof(VkPhysicalDeviceFeatures, sparseResidency2Samples)},
	{offsetof(VkPhysicalDeviceFeatures, sparseResidency4Samples)},
	{offsetof(VkPhysicalDeviceFeatures, sparseResidency8Samples)},
	{offsetof(VkPhysicalDeviceFeatures, sparseResidency16Samples)},
	{offsetof(VkPhysicalDeviceFeatures, sparseResidencyAliased)},
	{offsetof(VkPhysicalDeviceFeatures, variableMultisampleRate)},
	{offsetof(VkPhysicalDeviceFeatures, inheritedQueries)}
};


