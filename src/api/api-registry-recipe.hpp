#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "actor/components/particle-effect.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>
#include <api/api-registry-general.hpp>
#include <api/api-registry-item.hpp>
#include <item/recipe.hpp>

using std::string,std::variant;

namespace API {


    

    void loadRecipe(sol::table table,Recipe& recipe,Registry& registry) {
        getItemStack(table,"result",recipe.result,registry,true);
        get<float>(table,"time",recipe.time,true);
        get<string>(table,"category",recipe.category,true);
        sol::table ingredients = table["ingredients"];
        if(ingredients == sol::lua_nil) {
            Debug::warn("recipe has no ingredients");
        }
        int i = 1;
        recipe.ingredients.clear();
        while(i < 50) {
            sol::object element = ingredients[i];
            if(element == sol::lua_nil) {
                std::cout << "exiting " << std::endl;
                break;
            }
            ItemStack stack;
            getItemStack(ingredients,i,stack,registry,true);
            recipe.ingredients.push_back(stack);
            std::cout << "ingredient: " << stack.item->name << " x" << stack.amount << std::endl;
            i++;
        }
        if(i == 50) {
            Debug::warn("ingredient list did not exit or recipe was over 50 ingredients");
        }
    }

    struct RecipeRegistry {
        RecipeRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        Recipe* index(string name) {
            
            return registry.getRecipe(name);
        }
        
        void newindex(sol::this_state lua,string name,sol::object obj) {
            Debug::addTrace("recipes");
            Debug::addTrace(name);
            
            
            sol::table table = obj; //this will be null if it doesnt work, so be careful
            Recipe recipe;
            switch (getObjectLoadType(obj)) {
                
                case ObjLoadType::ARRAY:
                    Debug::warn("loading recipes from arrays is not supported");
                    break;
                case ObjLoadType::TABLE:
                    loadRecipe(table,recipe,registry);
                    registry.addRecipe(name,recipe);
                    break;
                case ObjLoadType::INVALID:
                    Debug::warn("trying to load with invalid object");
                    break;
                    
            }
            Debug::info("Loaded Recipe \"" + name + "\"",InfoPriority::MEDIUM);
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };

}