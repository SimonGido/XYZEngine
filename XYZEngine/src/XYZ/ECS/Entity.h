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

	private:
		uint32_t m_ID;

	};
}