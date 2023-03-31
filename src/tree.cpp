#include "tree.hpp"

using namespace cgp;


mesh create_cylinder_mesh(float radius, float height)
{
    mesh m;

    const double MIN_DISTANCE = 0.03;
    const int VERTICES = std::max(15, (int)(radius / MIN_DISTANCE));

    // Mesh
    for (int i = 0; i < VERTICES; i++) {
        double const a = 2 * Pi * (float) i / (float) VERTICES;
        m.position.push_back(vec3{ radius * cos(a), radius * sin(a), 0 });
        m.position.push_back(vec3{ radius * cos(a), radius * sin(a), height });
    }

    // Connectivity
    for (int i = 0; i < VERTICES ; i++) {
        int const s = 2 * i;
        m.connectivity.push_back(uint3{ (s + 1) % m.position.size(), (s) % m.position.size(), (s + 2) % m.position.size() });
        m.connectivity.push_back(uint3{ (s + 1) % m.position.size(), (s + 2) % m.position.size(), (s + 3) % m.position.size() });
    }


    // Need to call fill_empty_field() before returning the mesh 
    //  this function fill all empty buffer with default values (ex. normals, colors, etc).
    m.fill_empty_field();

    return m;
}

mesh create_cone_mesh(float radius, float height, float z_offset)
{
    mesh m;

    const double MIN_DISTANCE = 0.03;
    const int VERTICES = std::max(15, (int)(radius / MIN_DISTANCE));

    // Circle
    for (int i = 0; i < VERTICES; i++) {
        double a = 2 * Pi * (float)i / (float)VERTICES;
        m.position.push_back(vec3{ radius * cos(a), radius * sin(a), z_offset });
    }

    // Bottom and up vertices
    m.position.push_back(vec3{ 0, 0, z_offset });
    m.position.push_back(vec3{ 0, 0, z_offset + height });

    // Connectivity
    for (int i = 0; i < VERTICES - 1; i++) {
        m.connectivity.push_back(uint3{ VERTICES, i, i + 1 });
        m.connectivity.push_back(uint3{ VERTICES + 1, i , i + 1 });
    }
    m.connectivity.push_back(uint3{ VERTICES, VERTICES - 1, 0 });
    m.connectivity.push_back(uint3{ VERTICES + 1, VERTICES - 1, 0 });

    m.fill_empty_field();
    return m;
}

mesh create_tree()
{
    float h = 0.7f; // trunk height
    float r = 0.1f; // trunk radius

    // Create a brown trunk
    mesh trunk = create_cylinder_mesh(r, h);
    trunk.color.fill({ 0.4f, 0.3f, 0.3f });

    // Create a green foliage from 3 cones
    mesh foliage = create_cone_mesh(4 * r, 6 * r, 0.0f);      // base-cone
    foliage.push_back(create_cone_mesh(4 * r, 6 * r, 2 * r));   // middle-cone
    foliage.push_back(create_cone_mesh(4 * r, 6 * r, 4 * r));   // top-cone
    foliage.apply_translation_to_position({ 0,0,h });       // place foliage at the top of the trunk
    foliage.color.fill({ 0.4f, 0.6f, 0.3f });

    // The tree is composed of the trunk and the foliage
    mesh tree = trunk;
    tree.push_back(foliage);

    return tree;
}