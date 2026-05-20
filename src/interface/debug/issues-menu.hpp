#include "imgui/imgui.h"
#include "engine/debug.hpp"

namespace DebugMenu {
    inline void issuesMenu() {
        auto warnings = Debug::getWarnings();

        if(warnings.size() > 0) {
            ImGui::Begin("Issues");

            for (auto str : warnings)
            {
                ImGui::TextColored(ImVec4(1.0f,1.0f,0.2f,1.0f),str.c_str());
            }
            

            ImGui::End();
        }
    }
}