

entity = Entity.FindEntity("Test Entity")

function OnCreate()


end

function OnUpdate(ts)

	transform = entity:GetTransform()

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