#pragma once

#include <interface/interface.hpp>
#include "item/item-stack.hpp"
#include "text-widget.hpp"
#include "interface/widget.hpp"

template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

class TooltipWidget : public Widget {

    public:

        TextWidget* textWidget;
        Sprite backgroundSprite;
        Color backgroundColor = {};
        float padding = 3;
        float spacing = 2;
        float margin = 3;
       
    
        // returns if mouse inside
        void draw(DrawContext context,TooltipTarget target) {

            if(textWidget == nullptr) {
                Debug::warn("no text widget (tooltip)");
                return;
            }

            std::vector<string> lines;
            

            const auto visitor = overloads {
                [] (std::monostate m) {
                },
                [&] (ItemStack stack) {
                    if(stack.isEmpty()) return;
                    lines.push_back(stack.item->displayName);
                },
                [&] (Recipe* recipe) {
                    if(recipe == nullptr) return;
                    lines.push_back("CRAFT " + recipe->result.item->displayName);
                    for(auto ingredient : recipe->ingredients) {
                        if(ingredient.isEmpty()) continue;
                        lines.push_back(std::format(" - x{} {}",ingredient.amount,ingredient.item->displayName));
                    }
                }
            };

            

            

            std::visit(visitor,target);

            

            int longestLine = 0;
            for(auto line : lines) {
                longestLine = std::max(static_cast<int>(line.size()),longestLine);
            }

            if(lines.size() == 0) return;

            Rect tooltip = Rect(context.getMousePosition()+vec2(margin),vec2(0,0));

            // would be nice to turn this into some kinda vertical layout helper
            tooltip.size = textWidget->getSize(longestLine);
            tooltip.size.y += spacing;
            tooltip.size.y *= static_cast<int>(lines.size());
            tooltip.size.y -= spacing;
            tooltip.size += vec2(padding*2.0f);
            context.drawRect(tooltip,backgroundSprite,backgroundColor); // background

            for (auto line : lines)
            {
                textWidget->draw(context,tooltip.position + vec2(padding),line); // text
                tooltip.position.y += textWidget->getSize(line).y + spacing;
            }
            

        }

        string getTypeName() override {
            return "tooltip_widget";
        }

};