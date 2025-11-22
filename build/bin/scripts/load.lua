print("Loading Materials")

-- materials.cow = {shaders.lit,textures.cow}

materials.grid = {
    --shader=shaders.lit,
    texture=textures.grid_dark,
}

-- materials.container = {
--     shader="lit",
--     texture="container",
-- }

-- materials.white = {
--     shader="lit",
--     texture="solid",
--     color=color(0.1,0.5,0.8)
-- }

materials.pickaxe = {
    texture=textures.pickaxe_smooth,
}

materials.tin_block = {
    texture=textures.tin_block,
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
    model="block",
    material="tin_block"
}

blocks.cockpit = {
    type = "cockpit",
    model = "cockpit",
    material = "cockpit"
}

blocks.thruster = {
    type = "thruster",
    model = "thruster",
    material = "thruster",
    force = 20
}

blocks.furnace = {
    type = "furnace",
    model = "block",
    material = "thruster"
}


-- function setBlock(x,y,z)
--     construction:setBlock(ivec3(x,y,z),1);
-- end

-- function setBounds(x1,y1,z1,x2,y2,z2)
--     construction:setBounds(ivec3(x1,y1,z1),ivec3(x2,y2,z2));
-- end

-- function rotate(angle)
--     construction:rotate(vec3(0,angle,0))
-- end