
#pragma once

#include "interface/interface.hpp"
#include "helper/generic-storage.hpp"
#include "interface/widget.hpp"
#include "text-widget.hpp"
#include "interface/panel-widget.hpp"
#include "block/battery-block.hpp"


class BatteryPreviewWidget : public BlockWidget<BatteryBlock> {

    public:

    TextWidget* textWidget;
    PanelWidget mainPanel;
        


    void draw(DrawContext context,Construction* construction,Character& user,BatteryBlock& block,BlockStorage& storage) {

        Rect mainPanelRect = mainPanel.draw(context);
        
        int currentCharge = construction->getNetwork(0).getCurrentCharge();
        int maxCharge = construction->getNetwork(0).getMaxCharge();
        string text = std::format("{}/{}",currentCharge,maxCharge);
        TextDisplaySettings settings;
        settings.pivot = vec2(0.5f,0.5f);
        textWidget->draw(context,mainPanelRect.center(),text,settings);

    }

    string getTypeName() override {
        return "battery_preview_widget";
    }

    
};