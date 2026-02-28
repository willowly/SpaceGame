print("HIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII")

player:give(items.tin_ore,10)
player:give(items.pickaxe,1)
player:give(items.furnace,1)
player:give(items.thruster,5)
player:give(items.cockpit,1)
player:give(items.tin_plate,80)

print(player);

player:setToolbar(item_stack.new(items.pickaxe,1),0)