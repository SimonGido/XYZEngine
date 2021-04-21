#pragma once
#include "XYZ/Utils/DataStructures/MemoryPool.h"
#include "XYZ/Utils/DataStructures/Tree.h"

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
		String,
		Tree
	};

	enum class bUICallbackType
	{
		Hoover,
		Active
	};

	struct bUIElement;
	using bUICallback = std::function<void(bUICallbackType, bUIElement&)>;

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
		virtual ~bUIElement() = default;

		virtual void PushQuads(bUIRenderer& renderer, uint32_t& scissorID) {};
		virtual void OnUpdate();
		virtual bool OnMouseMoved(const glm::vec2& mousePosition);
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition);
		virtual bool OnRightMousePressed(const glm::vec2& mousePosition);
		virtual bool OnLeftMouseReleased() { return false; };
		virtual bool OnMouseScrolled(const glm::vec2& mousePosition, const glm::vec2& offset) { return false; }

		virtual glm::vec2 GetAbsolutePosition() const;
		void HandleVisibility(uint32_t scissorID);

		glm::vec2			 Coords;
		glm::vec2			 Size;
		glm::vec4			 Color;
		glm::vec4			 ActiveColor;
		std::string			 Label;
		std::string			 Name;
		bUIElement*			 Parent;
		bool				 Visible;
		bool				 ChildrenVisible;
		const bUIElementType Type;

		std::vector<bUICallback> Callbacks;


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
		virtual bool OnKeyPressed(int32_t mode, int32_t key) { return false; }
		virtual bool OnKeyTyped(char character) { return false; }

		static bUIListener* GetListener() { return s_Selected; }

		glm::vec2 Borders = glm::vec2(10.0f);
		bool FitText = true;
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
		virtual bool OnMouseMoved(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnRightMousePressed(const glm::vec2& mousePosition) override;
		virtual bool OnLeftMouseReleased() override { ResizeFlags = 0; return false; };

		glm::vec2 ButtonSize = { 25.0f, 25.0f };

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

		bool FitParent = true;
	};

	struct bUITreeItem
	{
		bUITreeItem(const std::string& label)
			: Label(label) {};

		std::string Label;
		glm::vec4	Color = glm::vec4(1.0f);
		bool		Open = false;

		const glm::vec2& GetCoords() const { return Coords;  }
		uint32_t GetKey() const { return Key; }
	private:
		glm::vec2	Coords = glm::vec2(0.0f);
		uint32_t    Key = 0;
		int32_t		ID = -1;
		
		friend class bUITree;
	};

	class bUITree : public bUIElement
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

		void AddItem(uint32_t key, uint32_t parent, const bUITreeItem& item);
		void AddItem(uint32_t key, const bUITreeItem& item);
		void RemoveItem(uint32_t key);
		void Clear();

		bUITreeItem& GetItem(uint32_t key);
		std::function<void(uint32_t)> OnSelect;

	private:
		void solveTreePosition();

	private:
		Tree Hierarchy;
		MemoryPool Pool;

		std::unordered_map<uint32_t, int32_t> NameIDMap;

		static constexpr size_t sc_NumberOfItemsPerBlockInPool = 10;
		static constexpr float sc_NodeOffset = 25.0f;

		friend class bUIRenderer;
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
}

