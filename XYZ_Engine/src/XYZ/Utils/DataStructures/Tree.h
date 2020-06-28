#pragma once

namespace XYZ {
	
	template <typename T>
	class Tree;

	template <typename T>
	class Node
	{	
		friend class Tree<T>;
	public:
		Node(const T& data)
			:
			m_Data(data)
		{}
	
		bool HasChild() const
		{
			return m_FirstChild != sc_NullIndex;
		}

		bool HasNextSibling() const
		{
			return m_NextSibling != sc_NullIndex;
		}

		bool HasPreviousSibling() const
		{
			return m_PreviousSibling != sc_NullIndex;
		}

		uint16_t GetParentIndex() const { return m_Parent; }

		const T& GetData() const { return m_Data; }

	private:
		void setIndex(uint16_t index)
		{
			m_Index = index;
		}

		void setParent(uint16_t parent, std::vector<Node<T>>& vector)
		{
			// Assing parent
			m_Parent = parent;

			// Assign to first child previous sibling this
			uint16_t firstChildIndex = vector[parent].m_FirstChild;
			if (firstChildIndex != sc_NullIndex)
			{
				vector[firstChildIndex].m_PreviousSibling = m_Index;
			}
			// Set as next sibling current first child
			m_NextSibling = vector[parent].m_FirstChild;

			// Set as first child this
			vector[parent].m_FirstChild = m_Index;
		}

		template <typename Setup>
		void setParent(uint16_t parent, std::vector<Node<T>>& vector, Setup& func )
		{
			// Assing parent
			m_Parent = parent;

			// Assign to first child previous sibling this
			uint16_t firstChildIndex = vector[parent].m_FirstChild;
			if (firstChildIndex != sc_NullIndex)
			{
				vector[firstChildIndex].m_PreviousSibling = m_Index;
			}
			// Set as next sibling current first child
			m_NextSibling = vector[parent].m_FirstChild;

			// Set as first child this
			vector[parent].m_FirstChild = m_Index;

			// Call user setup
			func(vector[m_Parent].m_Data, m_Data);
		}
		void detachFromTree(std::vector<Node<T>>& vector)
		{
			// If previous sibling exist assign to its next sibling this next sibling
			if (vector[m_Index].m_PreviousSibling != sc_NullIndex)
			{
				uint16_t previous = vector[m_Index].m_PreviousSibling;
				vector[previous].m_NextSibling = vector[m_Index].m_NextSibling;
			}
			else
			{
				// If previous sibling does not exist assign this next sibling directly as first child of parent
				vector[m_Parent].m_FirstChild = vector[m_Index].m_NextSibling;
			}
			m_Parent = sc_NullIndex;
			m_NextSibling = sc_NullIndex;
		}

		void reconnectToTree(std::vector<Node<T>>& vector, uint16_t newIndex)
		{
			// Reconnect with previous sibling
			if (vector[m_Index].m_PreviousSibling != sc_NullIndex)
			{
				uint16_t previous = vector[m_Index].m_PreviousSibling;
				vector[previous].m_NextSibling = newIndex;
			}
			else
			{
				// If not previous sibling connect directly to parent
				vector[m_Parent].m_FirstChild = newIndex;
			}
			// Reconnect with next sibling
			if (vector[m_Index].m_NextSibling != sc_NullIndex)
			{
				uint16_t next = vector[m_Index].m_NextSibling;
				vector[next].m_PreviousSibling = newIndex;
			}
			
			// Reconnect with children
			uint16_t child = vector[m_Index].m_FirstChild;
			while (child != sc_NullIndex)
			{
				vector[child].m_Parent = newIndex;
				child = vector[child].m_NextSibling;
			}

			m_Index = newIndex;
		}


	public:
		static constexpr uint16_t sc_NullIndex = 65535;

	private:
		T m_Data;
		uint16_t m_Index = sc_NullIndex;
		uint16_t m_Parent = sc_NullIndex;
		uint16_t m_FirstChild = sc_NullIndex;
		uint16_t m_PreviousSibling = sc_NullIndex;
		uint16_t m_NextSibling = sc_NullIndex;
	};


	template <typename T>
	class Tree
	{
	public:
		void Reserve(size_t count)
		{
			m_Data.reserve(count);
		}
		uint16_t InsertNode(const Node<T>& node)
		{
			XYZ_ASSERT(m_Data.size() < sc_MaxNumberOfElements, "Maximum number of elements in tree is ", sc_MaxNumberOfElements);
			m_Data.push_back(node);
			uint16_t index = m_Data.size() - 1;
			m_Data[index].setIndex(index);
			return index;
		}
		void SetRoot(uint16_t root)
		{
			XYZ_ASSERT(root < m_Data.size(), "Index out of range");
			m_Root = root;
		}

		void SetParent(uint16_t parent, uint16_t child)
		{
			XYZ_ASSERT(parent < m_Data.size() && child < m_Data.size(), "Index out of range");
			m_Data[child].setParent(parent, m_Data);
		}

		template<typename Func>
		void SetParent(uint16_t parent, uint16_t child, Func& func)
		{
			XYZ_ASSERT(parent < m_Data.size() && child < m_Data.size(), "Index out of range");
			m_Data[child].setParent(parent,m_Data,func);
		}
		
		void DeleteNode(uint16_t index)
		{
			XYZ_ASSERT(index < m_Data.size(), "Index out of range");
			// Delete children and it's siblings
			uint16_t next = m_Data[index].m_FirstChild;
			while (next != Node<T>::sc_NullIndex)
			{
				uint16_t child = next;
				next = m_Data[next].m_NextSibling;
				DeleteNode(child);
			}
			// Detach from tree
			m_Data[index].detachFromTree(m_Data);

			uint16_t last = m_Data.size() - 1;
			m_Data[last].reconnectToTree(m_Data, index);

			auto it = m_Data.begin() + index;
			uint16_t parentIndex = m_Data[index].GetParentIndex();

			// Move last element to the place of old one
			*it = std::move(m_Data.back());
			// Pops last element
			m_Data.pop_back();
		}


		template <typename Function>
		void Propagate(Function& func,uint16_t index = Node<T>::sc_NullIndex)
		{
			if (index == Node<T>::sc_NullIndex)
				index = m_Root;
			
			while (index != Node<T>::sc_NullIndex)
			{
				if (m_Data[index].HasChild())
				{
					uint16_t firstChild = m_Data[index].m_FirstChild;
					propagate(m_Data[index].m_Data,firstChild,func);
				}
				index = m_Data[index].m_NextSibling;
			}
		}
		
		void RestartIterator() 
		{ 
			m_Iterator = &m_Data[m_Root];
			m_CurrentIndex = m_Root;
		}
		bool Next()
		{
			XYZ_ASSERT(m_CurrentIndex != Node<T>::sc_NullIndex, "Tree index out of range");
			if (m_Data[m_CurrentIndex].HasChild())
			{
				m_CurrentIndex = m_Data[m_CurrentIndex].m_FirstChild;
				m_Iterator = &m_Data[m_CurrentIndex];
				return true;
			}
			else if (m_Data[m_CurrentIndex].HasNextSibling())
			{
				m_CurrentIndex = m_Data[m_CurrentIndex].m_NextSibling;
				m_Iterator = &m_Data[m_CurrentIndex];
				return true;
			}
			
			return false;
		}


		Node<T>* GetIterator() { return m_Iterator; }
		Node<T>* GetElement(int16_t index) { return m_Data[index].GetData(); }

		size_t GetSize() const { return m_Data.size(); }

		std::vector<Node<T>>& GetFlatData() { return m_Data; }

	private:
		template <typename Function>
		void propagate(T& parentValue,uint16_t index,Function& func)
		{
			while (index != Node<T>::sc_NullIndex)
			{
				func(parentValue, m_Data[index].m_Data);
				if (m_Data[index].HasChild())
				{
					uint16_t firstChild = m_Data[index].m_FirstChild;
					propagate(m_Data[index].m_Data,firstChild,func);
				}
				index = m_Data[index].m_NextSibling;
			}
		}

	private:
		uint16_t m_CurrentIndex = 0;
		Node<T> *m_Iterator = nullptr;

		// Reference to vector of nodes
		std::vector<Node<T>> m_Data;

		// Root by default null
		uint16_t m_Root = sc_MaxNumberOfElements;
		// Last value is used as null indicator
		static constexpr uint16_t sc_MaxNumberOfElements = (2 ^ 16) - 1;
	};
}