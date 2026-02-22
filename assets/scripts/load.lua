dofile("scripts/load/materials.lua")
dofile("scripts/load/widgets.lua")


print("Loading Blocks")

blocks.tin = {
    type="connected",
    texture="basic_block_sheet"
}

blocks.cockpit = {
    type = "cockpit",
    mesh = "cockpit",
    texture = "cockpit"
}

blocks.thruster = {
    type = "thruster",
    mesh = "thruster",
    texture = "thruster",
    force = 20000,
    side_force = 4000
}

blocks.furnace = {
    type = "furnace",
    mesh = "furnace",
    texture = "furnace"
}

print("Loading Items")

items.stone = {
    type = "resource",
    name = "Stone",
    icon = "stone_item"
}

items.tin_plate = {
    type = "placetool",
    name = "Tin Plate",
    icon = "tin_plate",
    model = "block",
    material = "tin_block",
    block = "tin"
}

items.tin_ore = {
    type = "resource",
    name = "Tin Ore",
    icon = "tin_ore_item"
}

items.furnace = {
    type = "placetool",
    name = "Furnace",
    icon = "furnace_item",
    model = "furnace",
    material = "thruster",
    block = "furnace",
    place_direction = 2,
}

items.thruster = {
    type = "placetool",
    name = "Thruster",
    icon = "thruster_item",
    model = "block",
    material = "thruster",
    block = "thruster"
}

items.cockpit = {
    type = "placetool",
    name = "Cockpit",
    icon = "cockpit_item",
    model = "cockpit",
    material = "cockpit",
    block = "cockpit"
}

items.pickaxe = {
    type = "pickaxe",
    name = "Pickaxe",
    icon = "pickaxe_item",
    model = "pickaxe",
    material = "pickaxe",
    durability = 300,
    offset = vec3(0.2,-0.4,-0.5),
    rotation = quat(-5,90,-5)
}

blocks.furnace.drop = items.furnace
blocks.tin.drop = items.tin_plate
blocks.thruster.drop = items.thruster
blocks.cockpit.drop = items.cockpit


actors.plane = {
    model = "plane",
    material = "grid"
}

actors.cube = {
    model = "cube",
    material = "grid"
}