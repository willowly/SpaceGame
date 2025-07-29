
print("Loading Texture Settings")

textures.grid:setPointFiltering()
textures.grid_dark:setPointFiltering()


print("Loading Materials")

materials.cow = {shaders.lit,textures.cow}

materials.grid = {
    shader=shaders.lit,
    texture=textures.grid_dark,
}

materials.container = {
    shader="lit",
    texture="container",
}

materials.pickaxe = {"lit","pickaxe"}

materials.tin_block = {"lit","tin_block"}
materials.cobalt_block = {"lit","cobalt_block"}
materials.cockpit = {"lit","cockpit"}
materials.thruster = {"lit","solid",color(0.3,0.3,0.4)}

materials.terrain = {"lit","solid",color(0.2,0.2,0.8)}

print("Loading Actors")

actors.plane = {
    type="actor",
    model="plane",
    material=materials.grid
}

actors.pickaxe = {
    type="actor",
    model="pickaxe",
    material="pickaxe"
}

actors.asteroid = {
    type="actor",
    model="asteroid",
    material="grid"
}

print("Loading Blocks")

blocks.cobalt = {"block","block","cobalt_block"}
blocks.tin = {"block","block","tin_block"}

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


function setBlock(x,y,z)
    construction:setBlock(ivec3(x,y,z),1);
end

function setBounds(x1,y1,z1,x2,y2,z2)
    construction:setBounds(ivec3(x1,y1,z1),ivec3(x2,y2,z2));
end

function rotate(angle)
    construction:rotate(vec3(0,angle,0))
end