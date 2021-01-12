entity = Entity.Find("Test Entity");

function OnCreate()


end

function OnUpdate(ts)

	newPos = Math.Vec3()

	if Input.IsKeyPressed(Input.KeyW) then
		newPos.y = ts
	elseif Input.IsKeyPressed(Input.KeyA) then
		newPos.x = -ts
	elseif Input.IsKeyPressed(Input.KeyS) then
		newPos.y = -ts
	elseif Input.IsKeyPressed(Input.KeyD) then
		newPos.x = ts
	end

	pos = Entity.GetTranslation(entity)

	pos.x = pos.x + newPos.x
	pos.y = pos.y + newPos.y
	pos.z = pos.z + newPos.z

	Entity.SetTranslation(entity, pos.x, pos.y, pos.z)

end