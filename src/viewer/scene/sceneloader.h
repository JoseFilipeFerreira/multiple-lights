

#pragma once

#include <ospray/ospray_cpp/Group.h>
#include <ospray/ospray_cpp.h>
#include <ospray/ospray_cpp/Material.h>
#include <json/json.h>

using namespace ospray;
using namespace ospcommon::math;

struct model {
    static cpp::Group loadobj(const std::string& objFileName);
    static cpp::Material getMaterial(const std::string& name, Json::Value& materials);
};


