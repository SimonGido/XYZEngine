#pragma once
#include "XYZ/Utils/DataStructures/MemoryPool.h"
#include "XYZ/Utils/DataStructures/Tree.h"
#include "XYZ/Renderer/SubTexture.h"

#include <glm/glm.hpp>

namespace XYZ {
	
	enum class bUIElementType
	{
		None,
		Button,
		Checkbox,
		Slider,
		Window,
		Scrollbox,
		Float,
		Int,
		String,
		Tree,
		Image,
		Text,
		Dropdown,

		// Advanced Types
		Timeline
	};

	enum class bUICallbackType
	{
		Hoover,
		Active,
		StateChange
	};

	struct bUIElement;
	using bUICallback = std::function<void(bUICallbackType, bUIElement&)>;

	class bUIQueue;
	class bUIRenderer;
	struct bUIElement
	{
		bUIElement(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);
		bUIElement(const bUIElement& other);
		bUIElement(bUIElement&& other) noexcept;

		virtual ~bUIElement() = default;

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) {};
		virtual void OnUpdate();
		virtual bool OnMouseMoved(const glm::vec2& mousePosition);
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition);
		virtual bool OnRightMousePressed(const glm::vec2& mousePosition);
		virtual bool OnLeftMouseReleased() { return false; };
		virtual bool OnMouseScrolled(const glm::vec2& mousePosition, const glm::vec2& offset) { return false; }

		virtual glm::vec2 GetAbsolutePosition() const;
		virtual glm::vec2 GetSize() const;
		bool HandleVisibility(uint32_t scissorID);

		int32_t GetID() const { return ID; }

		glm::vec2			 Coords;
		glm::vec2			 Size;
		glm::vec4			 Color;
		glm::vec4			 ActiveColor;
		std::string			 Label;
		std::string			 Name;
		bUIElement*			 Parent;
		bool				 Visible;
		bool				 ChildrenVisible;
		bool				 Locked;
		bool				 BlockEvents;
		const bUIElementType Type;

		std::vector<bUICallback> Callbacks;

	protected:
		uint32_t ScissorID = 0;

	private:
		uint32_t depth();

		int32_t ID = -1;

		friend class bUILoader;
		friend class bUIData;
		friend class bUI;
		friend class bUIAllocator;
	};


	struct bUIListener
	{
		virtual ~bUIListener() = default;

		virtual bool OnKeyPressed(int32_t mode, int32_t key) { return false; }
		virtual bool OnKeyTyped(char character) { return false; }

		static bUIListener* GetListener() { return s_Selected; }

		glm::vec2 Borders = glm::vec2(10.0f);
		bool FitText = false;
		bool CutTextOutside = true;
	protected:
		bool Listen = false;

		static void setListener(bUIListener* listener);

	private:
		static bUIListener* s_Selected;
	};

	struct bUIButton : public bUIElement
	{
		bUIButton(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);
		bUIButton(const bUIButton& other);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
	};

	struct bUICheckbox : public bUIElement
	{
		bUICheckbox(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual void OnUpdate() override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;

		bool Checked;
	};

	struct bUISlider : public bUIElement
	{
		bUISlider(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMouseReleased() override;

		float Value;
	private:
		bool Modified;
	};

	struct bUIWindow : public bUIElement
	{
		bUIWindow(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual void OnUpdate();
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnRightMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMouseReleased() override { ResizeFlags = 0; return false; };
		virtual glm::vec2 GetSize() const override;

		glm::vec2 ButtonSize = { 25.0f, 25.0f };
		std::function<void(const glm::vec2&)> OnResize;
		bool FitParent = false;
		bool IsHoovered = false;
	private:
		enum ResizeFlags
		{
			Right  = BIT(0),
			Left   = BIT(1),
			Bottom = BIT(2)
		};
		uint8_t ResizeFlags = 0;
		static constexpr glm::vec2 sc_ResizeOffset = glm::vec2(5.0f);
	};

	struct bUIScrollbox : public bUIElement
	{
		bUIScrollbox(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual void OnUpdate();
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override { return false; };
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override { return false; };
		virtual bool OnRightMousePressed(const glm::vec2& mousePosition) override { return false; };
		virtual bool OnLeftMouseReleased() override { return false; };
		virtual bool OnMouseScrolled(const glm::vec2& mousePosition, const glm::vec2& offset) override;
		virtual glm::vec2 GetAbsolutePosition() const override;
		glm::vec2 GetAbsoluteScrollPosition() const;

		glm::vec2 Offset;
		float Speed = 5.0f;

		bool EnableScroll = true;
		bool FitParent = true;
	};

	struct bUIHierarchyItem
	{
		bUIHierarchyItem(const std::string& label)
			: Label(label) {};

		std::string Label;
		glm::vec4	Color = glm::vec4(1.0f);
		bool		Open = false;

		uint32_t		 GetKey() const { return Key; }
		int32_t			 GetID() const { return ID; }
		const glm::vec2& GetCoords() const { return Coords;  }
	private:
		glm::vec2	Coords = glm::vec2(0.0f);
		uint32_t    Key = 0;
		int32_t		ID = -1;
		
		friend class bUIHierarchyElement;
	};

	class bUIHierarchyElement
	{
	public:
		bUIHierarchyElement();
		bUIHierarchyElement(bUIHierarchyElement&& other) noexcept;

		void AddItem(uint32_t key, uint32_t parent, const bUIHierarchyItem& item, bool atEnd = false);
		void AddItem(uint32_t key, const bUIHierarchyItem& item, bool atEnd = false);
		void SetParent(uint32_t key, uint32_t newParent);

		void RemoveItem(uint32_t key);
		void Clear();

		bUIHierarchyItem& GetItem(uint32_t key);
	protected:
		void solveTreePosition(const glm::vec2& size);

	protected:
		Tree Hierarchy;
		MemoryPool Pool;

		std::unordered_map<uint32_t, int32_t> NameIDMap;

		static constexpr size_t sc_NumberOfItemsPerBlockInPool = 10;
		static constexpr float sc_NodeOffset = 25.0f;
	};

	class bUITree : public bUIElement,
					public bUIHierarchyElement
	{
	public:
		bUITree(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);
		bUITree(bUITree&& other) noexcept;

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnRightMousePressed(const glm::vec2& mousePosition) override;

		
		std::function<void(uint32_t)> OnSelect;


		friend class bUIRenderer;
	};

	class bUIDropdown : public bUIElement,
						public bUIHierarchyElement		
	{
	public:
		bUIDropdown(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);
		bUIDropdown(bUIDropdown&& other) noexcept;

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnRightMousePressed(const glm::vec2& mousePosition) override;

		std::function<void(uint32_t)> OnSelect;

		friend class bUIRenderer;
	};

	struct bUIDynamicHierarchyElement
	{
		int32_t NextSibling = -1;
		int32_t FirstChild = -1;
	};

	class bUIFloat : public bUIElement,
					 public bUIListener
	{
	public:
		bUIFloat(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnKeyPressed(int32_t mode, int32_t key) override;
		virtual bool OnKeyTyped(char character) override;

		void	    SetValue(float val);
		float	    GetValue() const;
		const char* GetBuffer() const { return Buffer; }

		static constexpr size_t BufferSize = 60;
	private:
		uint32_t	  InsertionIndex;
		char		  Buffer[BufferSize];	
	};

	class bUIInt : public bUIElement,
				   public bUIListener
	{
	public:
		bUIInt(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnKeyPressed(int32_t mode, int32_t key) override;
		virtual bool OnKeyTyped(char character) override;

		void	    SetValue(int32_t val);
		int32_t	    GetValue() const;
		const char* GetBuffer() const { return Buffer; }

		static constexpr size_t BufferSize = 60;
	private:
		uint32_t	  InsertionIndex;
		char		  Buffer[BufferSize];	
	};

	class bUIString : public bUIElement,
					  public bUIListener
	{
	public:
		bUIString(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnKeyPressed(int32_t mode, int32_t key) override;
		virtual bool OnKeyTyped(char character) override;

		void			   SetValue(const std::string& value);
		const std::string& GetValue() const { return Buffer; }

		static constexpr size_t BufferSize = 60;
	private:
		uint32_t		InsertionIndex;
		std::string		Buffer;
	};


	class bUIImage : public bUIElement
	{
	public:
		bUIImage(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);
		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual void OnUpdate();
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;


		Ref<SubTexture> ImageSubTexture;

		bool EnableHighlight = false;
		bool FitParent = true;
	};

	class bUIText : public bUIElement
	{
	public:
		bUIText(
			const glm::vec2& coords,
			const glm::vec2& size,
			const glm::vec4& color,
			const std::string& label,
			const std::string& name,
			bUIElementType type
		);

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) override;
		virtual glm::vec2 GetSize() const override;

		std::string Text;
	};
}

