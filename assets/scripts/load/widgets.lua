print("Loading Widgets")


widgets.text_default = {type = "text",height = 20,spacing = 2}

widgets.item_slot = {
    type = "item_slot",
    sprite = "item_slot",
    color = color(0.2,0.2,0.2),
}

widgets.toolbar_item_slot = {
    type = "item_slot",
    sprite = "item_slot",
    bar_sprite = "solid",
    padding = 11,
    color = color(0.0,0.0,0.0,0.0),
}

widgets.recipe_slot = {
    type = "item_slot",
    sprite = "recipe_slot",
    color = color(0.2,0.2,0.2),
}

widgets.toolbar = {
    type = "toolbar",
    item_slot_sprite = "item_slot";
    sprite = "tech_hotbar";
    selector_sprite =  "tech_hotbar_selector";
    item_slot = "toolbar_item_slot",
    selector_size = 95;
    slot_size = 75;
    slot_height = 137;
    slot_gap = 3;
}

widgets.inventory = {
    type = "inventory",
    background_sprite = "solid";
    tooltip_text_title = "text_default";
    item_slot = "item_slot";
    recipe_slot = "recipe_slot";
};

widgets.player_widget = {
    type = "inventory",
    background_sprite = "solid";
    tooltip_text_title = "text_default";
    item_slot = "item_slot";
    recipe_slot = "recipe_slot";
};
