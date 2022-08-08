#pragma once



namespace XYZ {
	template <typename Integral, typename Fl>
	struct SortKey
	{
		void Set(Fl flag, Integral num)
		{
			m_Key |= (num << flag);
		}

		Integral GetKey() const { return m_Key; }
	private:
		Integral m_Key = 0;
	};
}