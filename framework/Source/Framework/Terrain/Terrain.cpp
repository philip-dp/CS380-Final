/******************************************************************************/
/*!
\file		Terrain.cpp
\project	CS380/CS580 AI Framework
\author		Dustin Holmes
\summary	Map and layer management

Copyright (C) 2018 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************************/

#include <pch.h>
#include "Terrain.h"
#include "TerrainAnalysis.h"
#include <fstream>
#include "Core/Serialization.h"

namespace fs = std::filesystem;

namespace
{
    const float layerHeightStep = -0.00001f;
}

Color Terrain::baseColor = Colors::White;
Color Terrain::wallColor = Colors::Black;
Color Terrain::opennessColor = Colors::Cyan;
Color Terrain::totalVisibilityColor = Colors::Orange;
Color Terrain::cellVisibilityColor = Colors::Green;
Color Terrain::positiveOccupancyColor = Colors::Red;
Color Terrain::negativeOccupancyColor = Colors::Blue;
Color Terrain::agentVisionColor = Colors::Purple;
Color Terrain::fogColor = Colors::Gray;
Color Terrain::seekSearchColor = Colors::Red;
Color Terrain::seekDetectionColor = Colors::Aquamarine;
float Terrain::maxLayerAlpha = 0.9f;
const float Terrain::mapSizeInWorld = 100.0f;

float globalScalar = 1.0f;

Terrain::Terrain() :
    wallLayer("Walls", layerHeightStep * 9.0f),
    pathLayer("Pathfinding", layerHeightStep * 8.0f),
    opennessLayer("Openness", layerHeightStep * 7.0f),
    totalVisibilityLayer("Total Visibility", layerHeightStep * 6.0f),
    cellVisibilityLayer("Cell Visibility", layerHeightStep * 5.0f),
    occupancyLayer("Occupancy", layerHeightStep * 4.0f),
    agentVisionLayer("Agent Vision", layerHeightStep * 3.0f),
    fogLayer("Fog of War", layerHeightStep * 1.0f),
    seekLayer("Seek", layerHeightStep * 2.0f),
    currentMap(-1)  
{}

bool Terrain::initialize()
{
    std::cout << "    Initializing Terrain System..." << std::endl;

    const fs::directory_iterator dir(Serialization::mapsPath);

    for (auto && entry : dir)
    {
        if (fs::is_regular_file(entry) == true)
        {
            MapData data;
            if (Serialization::deserialize(data, fs::path(entry)) == true)
            {
                mapData.emplace_back(std::move(data));
            }
        }
    }

    Callback clearCB = std::bind(&Terrain::reset_path_layer, this);
    Messenger::listen_for_message(Messages::PATH_REQUEST_BEGIN, clearCB);

    return mapData.size() > 0;
}

void Terrain::shutdown()
{
    std::cout << "    Shutting Down Terrain System..." << std::endl;
}

void Terrain::generate_positions()
{
    const unsigned width = mapData[currentMap].width;
    const unsigned height = mapData[currentMap].height;

    const float offset = globalScalar * 0.5f;

    const float xOffset = mapSizeInWorld / static_cast<float>(width);
    const float zOffset = mapSizeInWorld / static_cast<float>(height);

    positions.resize(height);

    for (unsigned h = 0; h < height; ++h)
    {
        positions[h].resize(width);
        const float x = xOffset * h;

        for (unsigned w = 0; w < width; ++w)
        {
            const float z = zOffset * w;

            positions[h][w] = Vec3(x + offset, 0.0f, z + offset);
        }
    }
}

void Terrain::load_map_data(const fs::path &file)
{
    std::ifstream stream(file);

    if (stream)
    {
        int height = -1;
        int width = -1;

        stream >> height;
        stream >> width;

        if (height <= 0 || width <= 0)
        {
            std::cout << "Invalid map parameters in " << file << std::endl;
            return;
        }

        mapData.emplace_back(height, width);
        auto &map = mapData.back();

        for (int h = 0; h < height; ++h)
        {
            for (int w = 0; w < width; ++w)
            {
                int value = 0;
                stream >> value;

                map.data[h][w] = !!value;
            }
        }

    }
    else
    {
        std::cout << "Unable to open map file " << file << std::endl;
    }
}

void Terrain::load_map(unsigned mapIndex)
{
    clear_graph();
    const auto &map = mapData[mapIndex];

    // inject the map data into the wall layer
    wallLayer.populate_with_data(map.data);
    wallLayer.configure_bool(baseColor, wallColor);
    wallLayer.set_enabled(true);

    // reinitialize the other map layers
    reset_path_layer();

    configure_float_map_layer(opennessLayer, map.height, map.width, opennessColor, opennessColor);
    configure_float_map_layer(totalVisibilityLayer, map.height, map.width, totalVisibilityColor, totalVisibilityColor);
    configure_float_map_layer(cellVisibilityLayer, map.height, map.width, cellVisibilityColor, cellVisibilityColor);
    configure_float_map_layer(occupancyLayer, map.height, map.width, positiveOccupancyColor, negativeOccupancyColor);
    configure_float_map_layer(agentVisionLayer, map.height, map.width, agentVisionColor, agentVisionColor);
    configure_float_map_layer(fogLayer, map.height, map.width, fogColor, fogColor);
    configure_float_map_layer(seekLayer, map.height, map.width, seekSearchColor, seekDetectionColor);

    globalScalar = mapSizeInWorld / static_cast<float>(map.width);

    generate_positions();

    refresh_static_analysis_layers();

    gen_graph();
    
    Messenger::send_message(Messages::MAP_CHANGE);
}

void Terrain::configure_float_map_layer(MapLayer<float> &layer, int height, int width, const Color &color0, const Color &color1)
{
    Color c0 = color0;
    c0.w = maxLayerAlpha;

    Color c1 = color1;
    c1.w = maxLayerAlpha;

    layer.populate_with_value(height, width, 0.0f);
    layer.configure_float(c0, c1);
}

void Terrain::refresh_static_analysis_layers()
{
    if (opennessLayer.enabled == true)
    {
        analyze_openness(opennessLayer);
    }

    if (totalVisibilityLayer.enabled == true)
    {
        analyze_visibility(totalVisibilityLayer);
    }
}

void Terrain::reset_path_layer()
{
    pathLayer.populate_with_value(mapData[currentMap].height, mapData[currentMap].width, Color(Colors::White));
    pathLayer.configure_color(maxLayerAlpha);
}

int Terrain::get_map_height() const
{
    return mapData[currentMap].height;
}

int Terrain::get_map_width() const
{
    return mapData[currentMap].width;
}

unsigned Terrain::get_map_index() const
{
    return currentMap;
}

const Vec3 &Terrain::get_world_position(int row, int col) const
{
    return positions[row][col];
}

const Vec3 &Terrain::get_world_position(const GridPos &gridPos)
{
    return positions[gridPos.row][gridPos.col];
}

GridPos Terrain::get_grid_position(const Vec3 &worldPos) const
{
    const int row = static_cast<int>((worldPos.x) / mapSizeInWorld * mapData[currentMap].height);
    const int col = static_cast<int>((worldPos.z) / mapSizeInWorld * mapData[currentMap].height);
    return GridPos { row, col };
}

bool Terrain::is_wall(int row, int col) const
{
    return wallLayer.get_value(row, col);
}

bool Terrain::is_wall(const GridPos &gridPos) const
{
    return wallLayer.get_value(gridPos);
}

bool Terrain::is_valid_grid_position(int row, int col) const
{
    const auto &data = mapData[currentMap];
    return row >= 0 && row < data.height && col >= 0 && col < data.width;
}

bool Terrain::is_valid_grid_position(const GridPos & gridPos) const
{
    const auto &data = mapData[currentMap];
    return gridPos.row >= 0 && gridPos.row < data.height && gridPos.col >= 0 && gridPos.col < data.width;
}

void Terrain::set_color(int row, int col, const Color &color)
{
    pathLayer.set_value(row, col, color);
}

void Terrain::set_color(const GridPos &gridPos, const Color &color)
{
    pathLayer.set_value(gridPos, color);
}

void Terrain::goto_next_map()
{
    // move to the next map
    currentMap = (currentMap + 1) % mapData.size();


    load_map(currentMap);
}

bool Terrain::goto_map(unsigned mapNum)
{
    if (mapNum >= mapData.size())
    {
        std::cout << "Attempted to change to invalid map number: " << mapNum << std::endl;
        return false;
    }

    if (currentMap != mapNum)
    {
        currentMap = mapNum;
        load_map(currentMap);
    }
    
    return true;
}

size_t Terrain::num_maps() const
{
    return mapData.size();
}

const DirectX::SimpleMath::Plane &Terrain::get_terrain_plane() const
{
    static const DirectX::SimpleMath::Plane plane(Vec3(0.0f, layerHeightStep * 8.0f, 0.0f), Vec3::Up);
    return plane;
}

void Terrain::draw()
{
    auto &instancer = renderer->get_grid_renderer();

    wallLayer.draw(instancer, positions);
    pathLayer.draw(instancer, positions);
    opennessLayer.draw(instancer, positions);
    totalVisibilityLayer.draw(instancer, positions);
    cellVisibilityLayer.draw(instancer, positions);
    occupancyLayer.draw(instancer, positions);
    agentVisionLayer.draw(instancer, positions);
    seekLayer.draw(instancer, positions);
    fogLayer.draw(instancer, positions);
    draw_graph();
}

void Terrain::draw_debug()
{}

Terrain::MapData::MapData(int height, int width) : height(height), width(width)
{
    data.resize(height);

    for (auto && row : data)
    {
        row.resize(width, false);
    }
}

void Terrain::gen_graph()
{
    // Time
    auto start = std::chrono::high_resolution_clock::now();

    // Get walls in map
    for (int i{}; i < terrain->get_map_width(); ++i)
    {
        for (int j{}; j < terrain->get_map_height(); ++j)
        {
            if (terrain->is_wall(i, j))
            {
                GridPos wall;
                wall.row = i;
                wall.col = j;
                Walls.push_back(wall);

                // Wall vertices
                Vec3 tl = terrain->get_world_position(i, j);
                tl.x -= 2.5f;
                tl.z += 2.5f;
                WallVertices.push_back(tl);
                Vec3 tr = terrain->get_world_position(i, j);
                tr.x += 2.5f;
                tr.z += 2.5f;
                WallVertices.push_back(tr);
                Vec3 bl = terrain->get_world_position(i, j);
                bl.x -= 2.5f;
                bl.z -= 2.5f;
                WallVertices.push_back(bl);
                Vec3 br = terrain->get_world_position(i, j);
                br.x += 2.5f;
                br.z -= 2.5f;
                WallVertices.push_back(br);

                // Wall edges
                WallEdges.push_back(std::make_pair(tl, tr));
                WallEdges.push_back(std::make_pair(tl, bl));
                WallEdges.push_back(std::make_pair(bl, br));
                WallEdges.push_back(std::make_pair(tr, br));
            }
        }
    }

    

    // First join all vertices to each other, even if intersecting wall
    for (int i{}, k_start{ 4 }; i < WallVertices.size(); ++i, k_start += 4)
    {
        for (int j{}; j < 4; ++j)
        {
            for (int k{ k_start }; k < WallVertices.size(); ++k)
            {
                add_edge(WallVertices[i], WallVertices[k]);
            }
            ++i;
        }
        --i;
    }


    // Check if line segment intersects with wall edge, if true, remove line
    for (int i{}; i < Edges.size(); ++i)
    {
        bool intersect{};
        for (int j{}; j < WallEdges.size(); ++j)
        {
            if (!is_clear_path(Edges[i].start, Edges[i].end, WallEdges[j].first, WallEdges[j].second))
            {
                intersect = true;
                break;
            }
        }

        if (!intersect)
        {
            PathEdges.push_back(Edges[i]);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Duration: " << duration.count() << " seconds" << "\n";
    std::cout << "Wall edge count: " << WallEdges.size() << "\n";
    std::cout << "Path edge count: " << PathEdges.size() << "\n\n";

}

void Terrain::toggle_graph()
{
    showGraph = !showGraph;
}

void Terrain::add_edge(Vec3 start, Vec3 end)
{
    Edges.push_back(Edge(start, end));
}

void Terrain::clear_graph()
{
    Walls.clear();
    WallVertices.clear();
    WallEdges.clear();
    Edges.clear();
    PathEdges.clear();
    showGraph = false;
}

void Terrain::draw_graph()
{
    if (showGraph)
    {
        auto& dr = renderer->get_debug_renderer();

        for (Edge const& edge : PathEdges)
            dr.draw_line(edge.start, edge.end, DirectX::Colors::Red);
    }
}