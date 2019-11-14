/*
This file belongs to Ashes.
See LICENSE file in root folder
*/
#pragma once

#include "renderer/D3D11Renderer/D3D11RendererPrerequisites.hpp"

namespace ashes::d3d11
{
	class PhysicalDevice
	{
	public:
		PhysicalDevice( VkInstance instance
			, AdapterInfo adapterInfo );
		~PhysicalDevice();

		VkBool32 getPresentationSupport( uint32_t queueFamilyIndex )const;
		VkLayerPropertiesArray enumerateLayerProperties()const;
		VkExtensionPropertiesArray enumerateExtensionProperties( std::string const & layerName )const;
		VkPhysicalDeviceProperties const & getProperties()const;
		VkPhysicalDeviceMemoryProperties getMemoryProperties()const;
		VkPhysicalDeviceFeatures getFeatures()const;
		VkQueueFamilyPropertiesArray getQueueFamilyProperties()const;
		VkFormatProperties getFormatProperties( VkFormat fmt )const;

#ifdef VK_KHR_get_physical_device_properties2

		VkPhysicalDeviceFeatures2KHR const & getFeatures2()const;
		VkPhysicalDeviceProperties2KHR const & getProperties2()const;
		VkFormatProperties2KHR const & getFormatProperties2( VkFormat format )const;
		VkResult getImageFormatProperties2( VkPhysicalDeviceImageFormatInfo2KHR const & imageFormatInfo
			, VkImageFormatProperties2KHR & imageFormatProperties )const;
		std::vector< VkQueueFamilyProperties2KHR > getQueueFamilyProperties2()const;
		VkPhysicalDeviceMemoryProperties2KHR const & getMemoryProperties2()const;
		std::vector< VkSparseImageFormatProperties2KHR > getSparseImageFormatProperties2( VkPhysicalDeviceSparseImageFormatInfo2KHR const & formatInfo )const;

#endif

		inline IDXGIAdapter * getAdapter()const
		{
			return m_adapterInfo.adapter;
		}

		inline IDXGIAdapter1 * getAdapter1()const
		{
			return m_adapterInfo.adapter1;
		}

		inline IDXGIAdapter2 * getAdapter2()const
		{
			return m_adapterInfo.adapter2;
		}

		inline IDXGIOutput * getOutput()const
		{
			return m_adapterInfo.output;
		}

		inline D3D_FEATURE_LEVEL getFeatureLevel()const
		{
			return m_adapterInfo.featureLevel;
		}

		inline VkInstance getInstance()const
		{
			return m_instance;
		}

	private:
		void doInitialise();

	private:
		VkInstance m_instance;
		AdapterInfo m_adapterInfo;
		VkPhysicalDeviceFeatures m_features{};
		VkPhysicalDeviceProperties m_properties{};
		VkQueueFamilyPropertiesArray m_queueProperties{};
		mutable std::map< VkFormat, VkFormatProperties > m_formatProperties;
#ifdef VK_KHR_get_physical_device_properties2
		VkPhysicalDeviceFeatures2KHR m_features2{};
		VkPhysicalDeviceProperties2KHR m_properties2{};
		std::vector< VkQueueFamilyProperties2KHR > m_queueProperties2{};
		mutable std::map< VkFormat, VkFormatProperties2KHR > m_formatProperties2;
		std::vector< VkSparseImageFormatProperties2KHR > m_sparseImageFormatProperties2;
#endif
	};
}
