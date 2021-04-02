#pragma once
#include "IGElement.h"
#include "IGMeshFactory.h"
#include "IGAllocator.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Core/MemoryPool.h"

namespace XYZ {
	class IGWindow : public IGElement
	{
	public:
		IGWindow(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		~IGWindow();

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnLeftRelease(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData , uint32_t scissorIndex = 0) override;

		void HandleActions(const glm::vec2& mousePosition, const glm::vec2& mouseDiff, bool& handled);

		enum Flags
		{
			Initialized	 = BIT(0),
			Hoovered	 = BIT(1),
			Moved		 = BIT(2),
			LeftResize	 = BIT(3),
			RightResize	 = BIT(4),
			BottomResize = BIT(5),
			Collapsed	 = BIT(6),
			Docked		 = BIT(7)
		};
	

		uint8_t Flags = 0;
		IGDockNode* Node = nullptr;
		std::function<void(const glm::vec2&)> ResizeCallback;

		static constexpr float PanelHeight = 25.0f;
	};

	class IGImageWindow : public IGWindow
	{
	public:
		IGImageWindow(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;

		Ref<SubTexture> SubTexture;
	};

	class IGButton : public IGElement
	{
	public:
		IGButton(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;
	
	};
	class IGCheckbox : public IGElement
	{
	public:
		IGCheckbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;
		
		bool Checked = false;
	};
	class IGSlider : public IGElement
	{
	public:
		IGSlider(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnLeftRelease(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;
	

		float Value = 0.0f;
	private:
		bool Modified = false;
	};

	class IGText : public IGElement
	{
	public:
		IGText(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
	
		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;
		

		std::string Text = "Text";
	};

	class IGFloat : public IGElement
	{
	public:
		IGFloat(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnKeyType(char character, bool& handled) override;
		virtual bool OnKeyPress(int32_t mode, int32_t key, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;
	
		void SetValue(float val);
		float GetValue() const;
		const char* GetBuffer() const { return Buffer; }


		bool  Listen = false;
		static constexpr size_t BufferSize = 60;

	private:
		mutable float Value = 0.0f;
		uint32_t ModifiedIndex;
		char Buffer[BufferSize];
	};

	class IGInt : public IGElement
	{
	public:
		IGInt(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnKeyType(char character, bool& handled) override;
		virtual bool OnKeyPress(int32_t mode, int32_t key, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;

		void SetValue(int32_t val);
		int32_t GetValue() const;
		const char* GetBuffer() const { return Buffer; }


		bool  Listen = false;
		static constexpr size_t BufferSize = 60;

	private:
		mutable int32_t Value = 0;
		uint32_t ModifiedIndex;
		char Buffer[BufferSize];
	};

	class IGString : public IGElement
	{
	public:
		IGString(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnKeyType(char character, bool& handled) override;
		virtual bool OnKeyPress(int32_t mode, int32_t key, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;

		void SetValue(const std::string& val);
		std::string GetValue() const;
		const char* GetBuffer() const { return Buffer; }


		bool  Listen = false;
		static constexpr size_t BufferSize = 60;

	private:
		uint32_t ModifiedIndex;
		char Buffer[BufferSize];
	};

	struct IGTreeItem
	{
		IGTreeItem(const std::string& label)
			: Label(label) {};

		std::string Label;
		int32_t		ID = -1;
		uint32_t    Key = 0;
		bool		Open = false;
		glm::vec2	Position = glm::vec2(0.0f);
		glm::vec2	TextSize = glm::vec2(0.0f);
		glm::vec4	Color = glm::vec4(1.0f);
	};

	class IGTree : public IGElement
	{	
	public:
		IGTree(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;
	

		void AddItem(uint32_t key, uint32_t parent, const IGTreeItem& item);
		void AddItem(uint32_t key, const IGTreeItem& item);
		void RemoveItem(uint32_t key);
		void Clear();

		IGTreeItem& GetItem(uint32_t child);

		std::function<void(uint32_t)> OnSelect;

	private:
		Tree Hierarchy;
		MemoryPool Pool;
	
		std::unordered_map<uint32_t, int32_t> NameIDMap;

		static constexpr size_t NumberOfItemsPerBlockInPool = 10;

		friend class IGMeshFactory;
	};

	class IGDropdown : public IGElement
	{
	public:
		IGDropdown(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
	};

	class IGGroup : public IGElement
	{
	public:
		IGGroup(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;

		bool Open = false;
		bool AdjustToParent = true;

		static constexpr float PanelHeight = 25.0f;
	};

	class IGSeparator : public IGElement
	{
	public:
		IGSeparator(const glm::vec2& position, const glm::vec2& size);

		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;
	
		bool AdjustToParent = true;
	};

	class IGScrollbox : public IGElement
	{
	public:
		IGScrollbox(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnMouseScroll(const glm::vec2& mousePosition, float offset, bool& handled);
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;
		virtual glm::vec2 BuildMesh(IGMesh& mesh, IGRenderData& renderData, IGPool& pool, const IGElement& root, uint32_t scissorIndex = 0) override;


		bool AdjustToParent = true;

		glm::vec2 Offset = glm::vec2(0.0f);

		static constexpr float ScrollSpeed = 4.0f;
	};

	class IGImage : public IGElement
	{
	public:
		IGImage(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;


		Ref<SubTexture> SubTexture;
	};

	class IGPack : public IGElement
	{
	public:
		IGPack(const std::vector<IGHierarchyElement>& elements);
		
		virtual bool OnLeftClick(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnLeftRelease(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnMouseMove(const glm::vec2& mousePosition, bool& handled) override;
		virtual bool OnKeyType(char character, bool& handled) override;
		virtual bool OnKeyPress(int32_t mode, int32_t key, bool& handled) override;

		virtual glm::vec2 BuildMesh(IGMesh& mesh, IGRenderData& renderData, IGPool& pool, const IGElement& root, uint32_t scissorIndex = 0) override;
		virtual glm::vec2 GenerateQuads(IGMesh& mesh, IGRenderData& renderData, uint32_t scissorIndex = 0) override;


		void Rebuild(const std::vector<IGHierarchyElement>& elements);

		
		Tree& GetHierarchy();
		const Tree& GetHierarchy() const;
		size_t PoolSize() const;

		IGElement& operator[](size_t index);
		const IGElement& operator[](size_t index) const;

	private:
		IGPool Pool;
	};
}