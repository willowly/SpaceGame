#include "imgui/imgui.h"
#include "actor/terrain.hpp"

namespace DebugMenu {

    void generationMenu() {
        ImGui::Begin("Generation");

        ImGui::InputFloat("Radius",&generationSettings.radius);
        ImGui::InputFloat("Noise Scale",&generationSettings.noiseScale);
        ImGui::InputFloat("Noise Factor",&generationSettings.noiseFactor);
        ImGui::InputInt("Noise Octaves",&generationSettings.noiseOctaves);
        ImGui::InputFloat("Noise Gain",&generationSettings.noiseGain);
        ImGui::InputFloat("Noise Lacunarity",&generationSettings.noiseLacunarity);
        if(ImGui::Button("Regenerate")) {
            // terrainLoader.stop();
            // terrain->destroy(&world);
            // terrain = world.spawn(Terrain::makeInstance(terrainMaterial,generationSettings,vec3(0,0,0)));
            // closing = false;
            // terrainLoader.start();
        }
        ImGui::End();
    }
}