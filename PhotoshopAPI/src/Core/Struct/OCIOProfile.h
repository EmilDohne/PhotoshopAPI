#pragma once

#include <vector>
#include <filesystem>

#include <OpenColorIO/OpenColorIO.h>

#include "Macros.h"

namespace OCIO = OCIO_NAMESPACE;

PSAPI_NAMESPACE_BEGIN


/// Read-only struct representing an OCIO profile as expressed on a photoshop file.
struct OCIOProfile
{
	OCIOProfile() = default;
	OCIOProfile(
		std::string config,
		std::string working_space,
		std::string view_transform,
		std::string display_transform
	) : m_WorkingSpace(working_space),
		m_ViewTransform(view_transform),
		m_DisplayTransform(display_transform)
	{
		// Since this file may have been created on a different PC than the current one, it is possible that
		// the config file cannot be mapped in the current read context. This should not be a failure!
		try
		{
			m_Config = OCIO::Config::CreateFromFile(config.c_str());
		}
		catch (const OCIO::Exception& e)
		{
			PSAPI_LOG_WARNING(
				"OCIO",
				"Unable to create OCIO config from file/uri %s. Full error: %s",
				config.c_str(),
				e.what()
			);
			m_Config = nullptr;
		}
	};
	OCIOProfile(
		OCIO::ConstConfigRcPtr config, 
		std::string working_space, 
		std::string view_transform,
		std::string display_transform
	) : m_Config(config), 
		m_WorkingSpace(working_space), 
		m_ViewTransform(view_transform),
		m_DisplayTransform(display_transform) {};

	OCIO::ConstConfigRcPtr config() const noexcept { return m_Config; }
	std::string working_space() const noexcept { return m_WorkingSpace; }
	std::string view_transform() const noexcept { return m_ViewTransform; }
	std::string display_transform() const noexcept { return m_DisplayTransform; }

private:
	OCIO::ConstConfigRcPtr m_Config = nullptr;
	std::string m_WorkingSpace;
	std::string m_ViewTransform;
	std::string m_DisplayTransform;
};

PSAPI_NAMESPACE_END