#pragma once
#include "XYZ/Asset/Asset.h"
#include "XYZ/Core/Timestep.h"

namespace XYZ {


	class PluginInterface
	{
	public:
		virtual ~PluginInterface() = default;

		virtual void OnCreate() = 0;
		virtual void OnDestroy() = 0;

		virtual void OnUpdate(Timestep ts) = 0;
	};
}