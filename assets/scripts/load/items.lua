print("Loading Items")

items.stone = {
    type = "resource",
    display_name = "Stone",
    icon = "stone_item",
    model = "item_ore",
    material = "stone",
}


materials.tin_plate = {
    texture = "plate"
}
items.tin_plate = {
    type = "placetool",
    display_name = "Tin Plate",
    icon = "tin_plate",
    model = "item_plate",
    material = "tin_plate",
    block = "tin"
}

items.tin_ore = {
    type = "resource",
    display_name = "Tin Ore",
    icon = "tin_ore_item",
    model = "item_ore",
    material = "tin_ore",
}

items.furnace = {
    type = "placetool",
    display_name = "Furnace",
    icon = "furnace_item",
    model = "furnace",
    material = "furnace",
    block = "furnace",
    place_direction = 2,
}

materials.thruster = {"lit","thruster"}
items.thruster = {
    type = "placetool",
    display_name = "Thruster",
    icon = "thruster_item",
    model = "thruster",
    material = "thruster",
    block = "thruster"
}

items.cockpit = {
    type = "placetool",
    display_name = "Cockpit",
    icon = "cockpit_item",
    model = "cockpit",
    material = "cockpit",
    block = "cockpit"
}

items.pickaxe = {
    type = "pickaxe",
    display_name = "Pickaxe",
    icon = "pickaxe_item",
    model = "pickaxe",
    material = "pickaxe",
    durability = 300,
    mine_amount = 5,
    offset = vec3(0.2,-0.4,-0.5),
    rotation = quat(-5,90,-5)
}


materials.pickaxe2 = {
    texture="pickaxe2"
}
items.drill = {
    type = "drill",
    display_name = "Drill",
    icon = "drill_item",
    model = "pickaxe",
    material = "pickaxe2",
    durability = 1000,
    mine_amount = 5,
    offset = vec3(0.2,-0.4,-0.5),
    rotation = quat(-5,90,-5)
}

blocks.furnace.drop = items.furnace
blocks.tin.drop = items.tin_plate
blocks.thruster.drop = items.thruster
blocks.cockpit.drop = items.cockpit