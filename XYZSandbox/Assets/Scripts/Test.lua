
position = Vec2()
entity = GetEntity("Test Entity");

function OnCreate()


end

function OnUpdate(ts)
	test = Vec2()
	test.x = ts
	test.y = ts

	test2 = GetTranslation(entity)

	position = test + test2

	SetTranslation(entity, position.x, position.y, 0)

end