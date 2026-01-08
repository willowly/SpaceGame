print("Loading Materials")


materials.grid = {
    --shader=shaders.lit,
    texture=textures.grid_dark,
}


materials.pickaxe = {
    texture=textures.pickaxe_smooth,
}

materials.tin_block = {
    texture=textures.tin_block,
}

materials.furnace = {
    texture="furnace"
}

materials.player = {
    texture = "solid"
}
-- materials.cobalt_block = {"lit","cobalt_block"}
materials.cockpit = {"lit","cockpit"}
materials.thruster = {"lit","solid",color(0.15,0.15,0.2)}

materials.terrain = {"lit","solid",color(0.2,0.2,0.8)}

-- print("Loading Actors")

actors.plane = {
    type="actor",
    model="plane",
    material=materials.grid
}

-- actors.pickaxe = {
--     type="actor",
--     model="pickaxe",
--     material="pickaxe"
-- }

actors.asteroid = {
    type="actor",
    model="asteroid",
    material="grid"
}

print("Loading Blocks")

-- blocks.cobalt = {"block","block","cobalt_block"}
blocks.tin = {
    type="block",
    model="connected",
    texture="block_sheet_clean"
}

blocks.cockpit = {
    type = "cockpit",
    model = "cockpit",
    texture = "cockpit"
}

blocks.thruster = {
    type = "thruster",
    model = "thruster",
    texture = "solid",
    force = 20
}

blocks.furnace = {
    type = "furnace",
    model = "furnace",
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
    placeDirection = 2,
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
