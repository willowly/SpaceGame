


textures.grid:setPointFiltering()
textures.grid_dark:setPointFiltering()


materials.cow = {shaders.lit,textures.cow}

materials.grid = {
    shader=shaders.lit,
    texture=textures.grid_dark,
}

materials.container = {
    shader="lit",
    texture="container",
}