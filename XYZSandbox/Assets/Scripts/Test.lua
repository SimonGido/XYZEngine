

entity = Entity.FindEntity("Test Entity")

function OnCreate()


end

function OnUpdate(ts)

	transform = entity:GetTransform()
	transform.Translation.x = transform.Translation.x + ts


end