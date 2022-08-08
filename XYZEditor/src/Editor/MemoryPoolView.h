#pragma once
#include "XYZ/Utils/DataStructures/MemoryPool.h"

namespace XYZ {
	namespace Editor {
		class MemoryPoolView
		{
		public:
			MemoryPoolView(const MemoryPool& pool);

			void OnImGuiRender();

		private:
			const MemoryPool& m_Pool;
		};
	}
}