require("Assets/Scripts/Another")

entity = Entity.FindEntity("Test Entity")
texture = Texture2D.Create(0, 0, 0, "Assets/Textures/player_sprite.png")


function OnCreate()


end

function OnUpdate(ts)

	transform = entity:GetTransform()
	spriteRenderer = entity:GetSpriteRenderer()
	animator = entity:GetAnimator()

	coords = Vec4(ts - 0.5, ts - 0.5, ts + 0.5, ts + 0.5)
	coords.x = Add(ts, 0.5)

	local subTexture = RefSubTexture.Create(texture, coords)
	
	spriteRenderer.Color.x = 1
	spriteRenderer.Color.y = 0
	spriteRenderer.Color.z = 0

	spriteRenderer.SubTexture = subTexture
	
	
	if Input.IsKeyPressed(65) then -- A
		transform.Translation.x = transform.Translation.x - ts * 5
		animator.Controller:TransitionTo(1)
	elseif Input.IsKeyPressed(68) then -- D
		transform.Translation.x = transform.Translation.x + ts * 5
		animator.Controller:TransitionTo(0)
	end
	
	if Input.IsKeyPressed(83) then -- S
		transform.Translation.y = transform.Translation.y - ts
	elseif Input.IsKeyPressed(87) then -- W
		transform.Translation.y = transform.Translation.y + ts
	end

end