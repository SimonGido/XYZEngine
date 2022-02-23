#pragma once

namespace XYZ {
	
	class Entity
	{
	public:
		Entity();
		Entity(uint32_t id);
		Entity(const Entity& other);

		Entity& operator =(const Entity& other);
		bool operator==(const Entity& rhs) const;
		bool operator!=(const Entity& rhs) const;

		operator uint32_t() const;	
		
		uint64_t GetHash() const;
	private:
		uint32_t m_ID;
		uint16_t m_Version;

		friend class EntityManager;
	};
}
namespace std {
	template<>
	struct hash<XYZ::Entity>
	{
		std::size_t operator()(XYZ::Entity entity) const noexcept
		{
			return entity.GetHash();
		}
	};
}
