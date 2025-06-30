
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
materials.chair = {"lit","solid",color(0.7,0.1,0.2)}
materials.thruster = {"lit","solid",color(0.3,0.3,0.4)}

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


function setBlock(x,y,z)
    construction:setBlock(ivec3(x,y,z),1);
end

function setBounds(x1,y1,z1,x2,y2,z2)
    construction:setBounds(ivec3(x1,y1,z1),ivec3(x2,y2,z2));
end

function rotate(angle)
    construction:rotate(vec3(0,angle,0))
end