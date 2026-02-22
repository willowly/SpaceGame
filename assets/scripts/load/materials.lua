print("Loading Materials")


materials.grid = {
    --shader=shaders.lit,
    texture=textures.grid_dark,
}

materials.tin_block = {
    texture=textures.tin_block,
}

materials.furnace = {
    texture="furnace"
}

materials.player = {
    texture = "player_face"
}
-- materials.cobalt_block = {"lit","cobalt_block"}
materials.cockpit = {"lit","cockpit"}
materials.thruster = {"lit","solid",color(0.15,0.15,0.2)}


materials.pickaxe = {
    texture="pickaxe_smooth"
}