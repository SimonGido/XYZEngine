#pragma once

namespace XYZ {
	
	template <typename T>
	static constexpr size_t Pow(T num, T pow)
	{
		size_t base = num;
		while (pow != 0)
		{
			num *= base;
			--pow;
		}
		return num;
	}


	template <typename T, typename IndexType>
	class Tree;

	template <typename T, typename IndexType>
	class Node
	{
		friend class Tree<T, IndexType>;
	public:
		Node(const T& data)
			:
			m_Data(data)
		{}
		bool HasChild() const { return m_FirstChild != sc_NullIndex; }
		bool HasNextSibling() const { return m_NextSibling != sc_NullIndex; }
		bool HasPreviousSibling() const { return m_PreviousSibling != sc_NullIndex; }
		IndexType GetIndex() const { return m_Index; }
		IndexType GetParentIndex() const { return m_Parent; }
		IndexType GetFirstChildIndex() const { return m_FirstChild;}
		IndexType GetNextSiblingIndex() const { return m_NextSibling;}
		IndexType GetPreviousSiblingIndex() const { return m_PreviousSibling;}
		const T& GetData() const { return m_Data;}
		T& GetData() { return m_Data;}

	private:
		void setParent(IndexType parent, std::vector<Node<T,IndexType>>& vector)
		{
			if (m_Parent == parent)
				return;

			m_Parent = parent;

			// Assign to first child previous sibling this
			IndexType firstChildIndex = vector[parent].m_FirstChild;
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
		void setParent(IndexType parent, std::vector<Node<T, IndexType>>& vector, Setup& func)
		{
			if (m_Parent == parent)
				return;

			if (m_Parent != sc_NullIndex)
			{
				if (m_PreviousSibling != sc_NullIndex)
					vector[m_PreviousSibling].m_NextSibling = m_NextSibling;
				else
					vector[m_Parent].m_FirstChild = m_NextSibling;
			}

			// Assign parent
			m_Parent = parent;

			// Assign to first child previous sibling this
			IndexType firstChildIndex = vector[parent].m_FirstChild;
			if (firstChildIndex != sc_NullIndex)
			{
				vector[firstChildIndex].m_PreviousSibling = m_Index;
			}
			// Set as next sibling current first child
			m_NextSibling = vector[parent].m_FirstChild;

			// Set as first child this
			vector[parent].m_FirstChild = m_Index;

			// Call user setup
			func(&vector[m_Parent], this);
		}
		void detachFromTree(std::vector<Node<T, IndexType>>& vector)
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

		void reconnectToTree(std::vector<Node<T, IndexType>>& vector, IndexType newIndex)
		{
			// Reconnect with previous sibling
			if (vector[m_Index].m_PreviousSibling != sc_NullIndex)
			{
				IndexType previous = vector[m_Index].m_PreviousSibling;
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
				IndexType next = vector[m_Index].m_NextSibling;
				vector[next].m_PreviousSibling = newIndex;
			}

			// Reconnect with children
			IndexType child = vector[m_Index].m_FirstChild;
			while (child != sc_NullIndex)
			{
				vector[child].m_Parent = newIndex;
				child = vector[child].m_NextSibling;
			}

			m_Index = newIndex;
		}


	public:
		static constexpr IndexType sc_NullIndex = Pow<IndexType>(2, sizeof(IndexType) * 8) - 1;

	private:
		T m_Data;
		IndexType m_Index = sc_NullIndex;
		IndexType m_Parent = sc_NullIndex;
		IndexType m_FirstChild = sc_NullIndex;
		IndexType m_PreviousSibling = sc_NullIndex;
		IndexType m_NextSibling = sc_NullIndex;
	};


	template <typename T, typename IndexType>
	class Tree
	{
	public:
		void Reserve(IndexType count)
		{
			m_Data.reserve(count);
		}
		IndexType InsertNode(const Node<T,IndexType>& node)
		{
			XYZ_ASSERT(m_Data.size() < sc_MaxNumberOfElements, "Maximum number of elements in tree is ", sc_MaxNumberOfElements);
			m_Data.push_back(node);
			IndexType index = m_Data.size() - 1;
			m_Data[index].m_Index = index;
			return index;
		}
		void SetRoot(IndexType root)
		{
			XYZ_ASSERT(root < m_Data.size(), "Index out of range");
			m_Root = root;
		}

		void SetParent(IndexType parent, IndexType child)
		{
			XYZ_ASSERT(parent < m_Data.size() && child < m_Data.size(), "Index out of range");
			m_Data[child].setParent(parent, m_Data);
		}

		template<typename Func>
		void SetParent(IndexType parent, IndexType child, Func& func)
		{
			XYZ_ASSERT(parent < m_Data.size() && child < m_Data.size(), "Index out of range");
			m_Data[child].setParent(parent, m_Data, func);
		}

		void DeleteNode(IndexType index)
		{
			XYZ_ASSERT(index < m_Data.size(), "Index out of range");
			// Delete children and it's siblings
			IndexType next = m_Data[index].m_FirstChild;
			while (next != Node<T, IndexType>::sc_NullIndex)
			{
				IndexType child = next;
				next = m_Data[next].m_NextSibling;
				DeleteNode(child);
			}
			// Detach from tree
			m_Data[index].detachFromTree(m_Data);

			IndexType last = m_Data.size() - 1;
			m_Data[last].reconnectToTree(m_Data, index);

			auto it = m_Data.begin() + index;
			IndexType parentIndex = m_Data[index].GetParentIndex();

			// Move last element to the place of old one
			*it = std::move(m_Data.back());
			// Pops last element
			m_Data.pop_back();
		}


		template <typename Function>
		void Propagate(Function& func, IndexType index = Node<T, IndexType>::sc_NullIndex)
		{
			if (index == Node<T, IndexType>::sc_NullIndex)
				index = m_Root;

			func(nullptr, &m_Data[index]);
			while (index != Node<T, IndexType>::sc_NullIndex)
			{
				if (m_Data[index].HasChild())
				{
					IndexType firstChild = m_Data[index].m_FirstChild;
					propagate(m_Data[index], firstChild, func);
				}
				index = m_Data[index].m_NextSibling;
			}
		}

		size_t GetSize() const
		{
			return m_Data.size();
		}


		std::vector<Node<T, IndexType>>& GetFlatData()
		{
			return m_Data;
		}

		const std::vector<Node<T, IndexType>>& GetFlatData() const
		{
			return m_Data;
		}

		T& operator [](IndexType index)
		{
			XYZ_ASSERT(index >= 0 && index < m_Data.size(), "Index out range");
			return m_Data[index].m_Data;
		}

		const T& operator [](IndexType index) const
		{
			XYZ_ASSERT(index >= 0 && index < m_Data.size(), "Index out range");
			return m_Data[index].m_Data;
		}
	private:
		template <typename Function>
		void propagate(Node<T, IndexType>& parentValue, IndexType index, Function& func)
		{
			while (index != Node<T, IndexType>::sc_NullIndex)
			{
				func(&parentValue, &m_Data[index]);
				if (m_Data[index].HasChild())
				{
					IndexType firstChild = m_Data[index].m_FirstChild;
					propagate(m_Data[index], firstChild, func);
				}
				index = m_Data[index].m_NextSibling;
			}
		}

	private:
		// Reference to vector of nodes
		std::vector<Node<T, IndexType>> m_Data;

		// Root by default null
		IndexType m_Root = Node<T, IndexType>::sc_NullIndex;
		// Last value is used as null indicator
		static constexpr IndexType sc_MaxNumberOfElements = Pow<IndexType>(2, sizeof(IndexType) * 8) - 1;
	};
}