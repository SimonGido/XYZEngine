

entity = LuaEntity("Test Entity")

function OnCreate()


end

function OnUpdate(ts)

	x = entity:GetTranslationX()
	x = x + ts 

	translation = entity:GetTranslation()

	entity:SetTranslation(x, 0, 0)

end