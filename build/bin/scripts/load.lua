
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

print("Loading Actors")

actors.plane = {
    type="actor",
    model="plane",
    material=materials.grid
}


function setBlock(x,y,z)
    construction:setBlock(ivec3.new(x,y,z),1);
end

function setBounds(x1,y1,z1,x2,y2,z2)
    construction:setBounds(ivec3.new(x1,y1,z1),ivec3.new(x2,y2,z2));
end

x = 0;

function place() 
    setBlock(x,0,0)
    x = x + 1;
end

function test()
    setBlock(0,1,0)
    setBlock(0,4,1)
    setBlock(0,4,2)
end