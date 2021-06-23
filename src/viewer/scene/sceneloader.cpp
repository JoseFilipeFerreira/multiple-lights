#include "sceneloader.h"
#include <json/value.h>
#include <ospcommon/math/vec.h>
#include <ospray/ospray_cpp/Material.h>
#include <stdexcept>
#include <fstream>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <json/json.h>

cpp::Group model::loadobj(const std::string& objFileName) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> obj_materials;
    std::string err, warn;

    std::string obj_base_dir  = objFileName.substr(0, objFileName.rfind('/') + 1);
    std::string mat_json_file = objFileName.substr(0, objFileName.rfind('.') + 1) + std::string("mat");
  
    auto ret =
        tinyobj::LoadObj(&attrib, &shapes, &obj_materials, &err, objFileName.c_str(), obj_base_dir.c_str(), true);

    if (!ret || !err.empty()) {
        throw std::runtime_error("TinyOBJ Error loading " + objFileName + " error: " + err);
    }

    Json::Value jsonmaterials;

    std::ifstream ifile(mat_json_file);

    bool OSPRayMaterialFile = false;

    if (ifile.is_open()) {

        ifile >> jsonmaterials;
        ifile.close();

        OSPRayMaterialFile = true;
    }

    std::vector<uint32_t> material_ids;

    std::vector<cpp::GeometricModel> _geometries;

    for (size_t s = 0; s < shapes.size(); ++s) {
        std::vector<vec3f> vertex;
        std::vector<vec3f> normals;
        std::vector<vec4f> color;
        std::vector<vec2f> uv;
        std::vector<vec3ui> findex;
        
        const tinyobj::mesh_t& obj_mesh = shapes[s].mesh;

        auto minmax_matid = std::minmax_element(obj_mesh.material_ids.begin(), obj_mesh.material_ids.end());

        if (*minmax_matid.first != *minmax_matid.second) {
            std::runtime_error(
                "Warning: per-face material IDs are not supported, materials may look "
                "wrong."
                " Please reexport your mesh with each material group as an OBJ group\n");
        }

        for (size_t f = 0; f < obj_mesh.num_face_vertices.size(); ++f) {
            if (obj_mesh.num_face_vertices[f] != 3) {
                throw std::runtime_error("Non-triangle face found in " + objFileName + "-" + shapes[s].name);
            }

            vec3ui tri_indices;
            for (size_t i = 0; i < 3; ++i) {
                const vec3i idx(obj_mesh.indices[f * 3 + i].vertex_index,
                                obj_mesh.indices[f * 3 + i].normal_index,
                                obj_mesh.indices[f * 3 + i].texcoord_index);

                uint32_t vert_idx = vertex.size();

                vertex.emplace_back(
                    attrib.vertices[3 * idx.x + 0], attrib.vertices[3 * idx.x + 1], attrib.vertices[3 * idx.x + 2]);

                if (idx.y != uint32_t(-1)) {
                    vec3f n(attrib.normals[3 * idx.y], attrib.normals[3 * idx.y + 1], attrib.normals[3 * idx.y + 2]);
                    normals.push_back(normalize(n));
                }

                if (idx.z != uint32_t(-1)) {
                    uv.emplace_back(attrib.texcoords[2 * idx.z], attrib.texcoords[2 * idx.z + 1]);
                }

                tri_indices[i] = vert_idx;
            }
            findex.push_back(tri_indices);
        }

        cpp::Geometry geom("mesh");
        geom.setParam("vertex.position", cpp::Data(vertex));
        geom.setParam("index", cpp::Data(findex));
        if (normals.size() == vertex.size())
            geom.setParam("vertex.normal", cpp::Data(normals));
        // geom.setParam("vertex.color", cpp::Data(color));

        geom.commit();

        cpp::GeometricModel model(geom);
        /********
         * The following comment disables json materials loading 
         * from .mat files
         *
         *  Uncomment to reactivate loading JASON material from .mat files
         ********/
         /*
        if (OSPRayMaterialFile) {
            //std::cout << "Took the OSPRayMaterialFile branch" << std::endl;
            model.setParam("material", getMaterial(obj_materials[obj_mesh.material_ids[0]].name, jsonmaterials));
        }
        else */{
            //std::cout << "Took the NON OSPRayMaterialFile branch" << std::endl;
            //std::cout << "Mat name is " << obj_materials[obj_mesh.material_ids[0]].name << std::endl; 
            auto ke          = vec3f(obj_materials[obj_mesh.material_ids[0]].emission[0],
                            obj_materials[obj_mesh.material_ids[0]].emission[1],
                            obj_materials[obj_mesh.material_ids[0]].emission[2]);
            auto lightsource = (length(ke) > 0);

            auto kd = vec3f(obj_materials[obj_mesh.material_ids[0]].diffuse[0],
                            obj_materials[obj_mesh.material_ids[0]].diffuse[1],
                            obj_materials[obj_mesh.material_ids[0]].diffuse[2]);
            auto ks = vec3f(obj_materials[obj_mesh.material_ids[0]].specular[0],
                            obj_materials[obj_mesh.material_ids[0]].specular[1],
                            obj_materials[obj_mesh.material_ids[0]].specular[2]);
            auto tf = vec3f(obj_materials[obj_mesh.material_ids[0]].transmittance[0],
                            obj_materials[obj_mesh.material_ids[0]].transmittance[1],
                            obj_materials[obj_mesh.material_ids[0]].transmittance[2]);

            auto ns = obj_materials[obj_mesh.material_ids[0]].shininess;
            auto d  = obj_materials[obj_mesh.material_ids[0]].dissolve;

            if (!lightsource) {
                auto mat = cpp::Material("pathtracer", "MyMaterial");
                mat.setParam("kd", kd);
                mat.setParam("ks", ks);
                mat.setParam("ns", ns);
                if (length(tf) > 0)
                    mat.setParam("d", length(tf));
                else
                    mat.setParam("d", d);
                mat.commit();
                model.setParam("material", mat);
                

            } else {
                auto mat = cpp::Material("pathtracer", "luminous");
                mat.setParam("color", kd);
                mat.setParam("transparency", tf[0]);
                mat.setParam("intensity", ke[0]);
                mat.commit();
                model.setParam("material", mat);
            }
        }
        model.commit();
        _geometries.push_back(model);
    }
    cpp::Group MeshGroup;

    MeshGroup.setParam("geometry", cpp::Data(_geometries));
    MeshGroup.commit();

    return MeshGroup;
}

cpp::Material model::getMaterial(const std::string& name, Json::Value& materials) {
    if (materials.isMember(name)) {
        auto& node = materials[name];
        cpp::Material mat("pathtracer", node["Type"].asString());

        std::cout << name << std::endl;

        for (auto& p : node.getMemberNames()) {

            if (p == "Type")
                continue;
    
            //std::cout << "\t" << p << std::endl;

            if (node[p].isDouble())
                mat.setParam(p, node[p].as<float>());
            else if (node[p].isInt()) {
                mat.setParam(p, node[p].as<int>());
            } else if (node[p].isBool()) {
                mat.setParam(p, node[p].as<bool>());
            } else if (node[p].isString()) {
                mat.setParam(p, node[p].asString());
            } else if (node[p].isArray()) {
                auto size = node[p].size();
                if (size == 3) {
                    mat.setParam(p, vec3f(node[p][0].as<float>(), node[p][1].as<float>(), node[p][2].as<float>()));
                } else if (size == 4) {
                    mat.setParam(p,
                                 vec4f(node[p][0].as<float>(),
                                       node[p][1].as<float>(),
                                       node[p][2].as<float>(),
                                       node[p][3].as<float>()));
                }
            }
        }
        mat.commit();
        return std::move(mat);
    }
    cpp::Material mat("pathtracer", "obj");
    mat.commit();
    return std::move(mat);
}
