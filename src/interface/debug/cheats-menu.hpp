#include "imgui/imgui.h"
#include "actor/character.hpp"
#include "engine/world.hpp"
#include "engine/registry.hpp"

namespace DebugMenu {

    inline void cheatsMenu(Character& player,World& world,Registry& registry) {
        ImGui::Begin("Cheats");

        // Give Items
        if(ImGui::Button("Give Items")) {
            ImGui::OpenPopup("GiveItemPopup");
        }

        if(ImGui::BeginPopup("GiveItemPopup")) {
            for (auto& pair : registry.getItems())
            {
                auto& item = pair.second;
                string name = item->displayName;
                if(name == "") {
                    name = item->name;
                }
                if(name == "") {
                    name = pair.first;
                }
                if(name == "") {
                    continue;
                }
                if(ImGui::Button(name.c_str())) {
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        player.give(ItemStack(item.get(),1));
                    }
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                        ImGui::OpenPopup(("GiveItemPopup_" + name).c_str());
                    }
                    if(ImGui::BeginPopup("GiveItemPopup")) {
                        if(ImGui::Button("Give x5")) {
                            player.give(ItemStack(item.get(),5));
                        }
                        if(ImGui::Button("Give x10")) {
                            player.give(ItemStack(item.get(),10));
                        }
                        if(ImGui::Button("Give x100")) {
                            player.give(ItemStack(item.get(),100));
                        }
                        ImGui::End();
                    }
                    player.give(ItemStack(item.get(),1));
                }
            }
            ImGui::End();
        }

        ImGui::End();
    }
}