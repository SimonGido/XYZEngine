

entity = Entity.FindEntity("Test Entity")

function OnCreate()


end

function OnUpdate(ts)

	transform = entity:GetTransform()
	spriteRenderer = entity:GetSpriteRenderer()

	spriteRenderer.Color.x = transform.Translation.x
	spriteRenderer.Color.y = transform.Translation.y

	coords = Vec4(ts + 0.5, ts + 0.5, ts - 0.5, ts - 0.5)
	coords.x = 5
	spriteRenderer.Color.z = coords.x


	if Input.IsKeyPressed(65) then
		transform.Translation.x = transform.Translation.x - ts
	elseif Input.IsKeyPressed(68) then
		transform.Translation.x = transform.Translation.x + ts
	end

	if Input.IsKeyPressed(83) then
		transform.Translation.y = transform.Translation.y - ts
	elseif Input.IsKeyPressed(87) then
		transform.Translation.y = transform.Translation.y + ts
	end

end