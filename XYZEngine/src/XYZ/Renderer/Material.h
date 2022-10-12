#pragma once
#include "Shader.h"
#include "Texture.h"
#include "MaterialInstance.h"

#include "XYZ/Core/Flags.h"
#include "XYZ/Utils/DataStructures/ByteBuffer.h"

#include <queue>
#include <bitset>
#include <unordered_set>


namespace XYZ {

	enum class RenderFlags : uint64_t
	{
		MaterialFlag	= 1ULL << 0,
		TransparentFlag = 1ULL << 16,
		InstancedFlag	= 1ULL << 17
	};


	class Material : public RefCount
	{
	public:
		using OnInvalidateFunc = std::function<void()>;

		virtual ~Material() {};

		virtual void Invalidate() = 0;
		
		virtual void SetFlag(RenderFlags renderFlag, bool val = true) = 0;
		virtual void SetImageArray(const std::string& name, Ref<Image2D> image, uint32_t arrayIndex) = 0;
		virtual void SetImage(const std::string& name, Ref<Image2D> image, int32_t mip = -1) = 0;

		virtual uint64_t	GetFlags() const = 0;
		virtual uint32_t	GetID()    const = 0;
		virtual Ref<Shader>	GetShader()const = 0;
		
		

		static Ref<Material> Create(const Ref<Shader>& shader);

		friend class MaterialInstance;
		friend class MaterialAsset;
	protected:
		void		    invalidateInstances();

		std::unordered_set<WeakRef<MaterialInstance>> m_MaterialInstances;
		OnInvalidateFunc m_OnInvalidate;
	};


}
