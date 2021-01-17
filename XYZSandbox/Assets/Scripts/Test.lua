require("Assets/Scripts/Another")

entity = Entity.FindEntity("Test Entity")

function OnCreate()


end

function OnUpdate(ts)

	transform = entity:GetTransform()
	spriteRenderer = entity:GetSpriteRenderer()
	--animator = entity:GetAnimator()

	spriteRenderer.Color.x = transform.Translation.x
	spriteRenderer.Color.y = transform.Translation.y

	coords = Vec4(ts + 0.5, ts + 0.5, ts - 0.5, ts - 0.5)
	coords.x = Add(ts, 0.5)
	spriteRenderer.Color.z = coords.x

	texture = Texture2D.Create(0, 0, 0, "Assets/Textures/bird.png")
	subTexture = SubTextureRef.Create(texture, coords)

	
	spriteRenderer.SubTexture = subTexture
	spriteRenderer.SubTexture:Get():SetTexture(texture)
	
	
	
	if Input.IsKeyPressed(65) then -- A
		transform.Translation.x = transform.Translation.x - ts * 5
		--animator.Controller:TransitionTo(1)
	elseif Input.IsKeyPressed(68) then -- D
		transform.Translation.x = transform.Translation.x + ts * 5
		--animator.Controller:TransitionTo(0)
	end
	
	if Input.IsKeyPressed(83) then -- S
		transform.Translation.y = transform.Translation.y - ts
	elseif Input.IsKeyPressed(87) then -- W
		transform.Translation.y = transform.Translation.y + ts
	end

end