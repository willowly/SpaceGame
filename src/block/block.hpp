#pragma once


#include <graphics/model.hpp>
#include <graphics/material.hpp>

class Block {

    public:
        Model* model;
        Material* material;
        bool canRide;

        Block(Model* model,Material* material) : model(model), material(material) {

        }

};