#include <pch.h>
#include "Terrain/TerrainAnalysis.h"
#include "Terrain/MapMath.h"
#include "Agent/AStarAgent.h"
#include "Terrain/MapLayer.h"
#include "Projects/ProjectThree.h"

#include <iostream>

bool ProjectThree::implemented_fog_of_war() const // extra credit
{
    return false;
}

float distance_to_closest_wall(int row, int col)
{
    /*
        Check the euclidean distance from the given cell to every other wall cell,
        with cells outside the map bounds treated as walls, and return the smallest
        distance.  Make use of the is_valid_grid_position and is_wall member
        functions in the global terrain to determine if a cell is within map bounds
        and a wall, respectively.
    */

    // WRITE YOUR CODE HERE
    
    return 0.0f; // REPLACE THIS
}

bool is_clear_path(int row0, int col0, int row1, int col1)
{
    // Check walls only within bounding box
    float minrow = (row0 > row1) ? static_cast<float>(row1) : static_cast<float>(row0);
    float maxrow = (row0 > row1) ? static_cast<float>(row0) : static_cast<float>(row1);
    float mincol = (col0 > col1) ? static_cast<float>(col1) : static_cast<float>(col0);
    float maxcol = (col0 > col1) ? static_cast<float>(col0) : static_cast<float>(col1);

    Vec2 startline = Vec2(static_cast<float>(row0) / 20.0f, static_cast<float>(col0) / 20.0f);
    Vec2 endline = Vec2(static_cast<float>(row1) / 20.0f, static_cast<float>(col1) / 20.0f);

    float offset = 1.0f / 40.0f + FLT_EPSILON;

    for (float i = minrow; i <= maxrow; ++i)
    {
        for (float j = mincol; j <= maxcol; ++j)
        {
            if (terrain->is_wall(static_cast<int>(i), static_cast<int>(j)))
            {
                // Check if line intersects any of 4 edges
                Vec2 tl = Vec2(static_cast<float>(i / 20.0f - offset), static_cast<float>(j / 20.0f - offset));
                Vec2 tr = Vec2(static_cast<float>(i / 20.0f + offset), static_cast<float>(j / 20.0f - offset));
                Vec2 bl = Vec2(static_cast<float>(i / 20.0f - offset), static_cast<float>(j / 20.0f + offset));
                Vec2 br = Vec2(static_cast<float>(i / 20.0f + offset), static_cast<float>(j / 20.0f + offset));

                if (line_intersect(startline, endline, tl, tr) ||
                    line_intersect(startline, endline, tl, bl) ||
                    line_intersect(startline, endline, bl, br) ||
                    line_intersect(startline, endline, tr, br))
                        return true;
                
            }
        }
    }

    return false;
}

bool is_clear_path(Vec3 const& s1, Vec3 const& e1, Vec3 s2, Vec3 e2)
{
    // Add/subtract s2 and e2 with epsilon
    float min{}, max{};
    float t{}, u{};
    if (std::abs(s2.x - e2.x) <= FLT_EPSILON) // Vertical wall
    {
        min = (s2.z > e2.z) ? e2.z : s2.z;
        max = (s2.z < e2.z) ? e2.z : s2.z;
        min -= FLT_EPSILON * 32.5f;
        max += FLT_EPSILON * 32.5f;

        t = ((s1.x - s2.x) * (max - min) - (s1.z - max) * (s2.x - e2.x)) / ((s1.x - e1.x) * (max - min) - (s1.z - e1.z) * (s2.x - e2.x));
        u = -(((s1.x - e1.x) * (s1.z - max) - (s1.z - e1.z) * (s1.x - s2.x)) / ((s1.x - e1.x) * (max - min) - (s1.z - e1.z) * (s2.x - e2.x)));

    }
    else if (std::abs(s2.z - e2.z) <= FLT_EPSILON) // Horizontal wall
    {
        min = (s2.x > e2.x) ? e2.x : s2.x;
        max = (s2.x < e2.x) ? e2.x : s2.x;
        min -= FLT_EPSILON * 32.5f;
        max += FLT_EPSILON * 32.5f;

        t = ((s1.x - max) * (s2.z - e2.z) - (s1.z - s2.z) * (max - min)) / ((s1.x - e1.x) * (s2.z - e2.z) - (s1.z - e1.z) * (max - min));
        u = -(((s1.x - e1.x) * (s1.z - s2.z) - (s1.z - e1.z) * (s1.x - max)) / ((s1.x - e1.x) * (s2.z - e2.z) - (s1.z - e1.z) * (max - min)));
    }


    if (t > 0.0f && t < 1.0f && u > 0.0f && u < 1.0f)
        return false;
    else return true;
    
}

void analyze_openness(MapLayer<float> &layer)
{
    /*
        Mark every cell in the given layer with the value 1 / (d * d),
        where d is the distance to the closest wall or edge.  Make use of the
        distance_to_closest_wall helper function.  Walls should not be marked.
    */

    // WRITE YOUR CODE HERE
}

void analyze_visibility(MapLayer<float> &layer)
{
    /*
        Mark every cell in the given layer with the number of cells that
        are visible to it, divided by 160 (a magic number that looks good).  Make sure
        to cap the value at 1.0 as well.

        Two cells are visible to each other if a line between their centerpoints doesn't
        intersect the four boundary lines of every wall cell.  Make use of the is_clear_path
        helper function.
    */

    // WRITE YOUR CODE HERE
}

void analyze_visible_to_cell(MapLayer<float> &layer, int row, int col)
{
    /*
        For every cell in the given layer mark it with 1.0
        if it is visible to the given cell, 0.5 if it isn't visible but is next to a visible cell,
        or 0.0 otherwise.

        Two cells are visible to each other if a line between their centerpoints doesn't
        intersect the four boundary lines of every wall cell.  Make use of the is_clear_path
        helper function.
    */

    // WRITE YOUR CODE HERE
}

void analyze_agent_vision(MapLayer<float> &layer, const Agent *agent)
{
    /*
        For every cell in the given layer that is visible to the given agent,
        mark it as 1.0, otherwise don't change the cell's current value.

        You must consider the direction the agent is facing.  All of the agent data is
        in three dimensions, but to simplify you should operate in two dimensions, the XZ plane.

        Take the dot product between the view vector and the vector from the agent to the cell,
        both normalized, and compare the cosines directly instead of taking the arccosine to
        avoid introducing floating-point inaccuracy (larger cosine means smaller angle).

        Give the agent a field of view slighter larger than 180 degrees.

        Two cells are visible to each other if a line between their centerpoints doesn't
        intersect the four boundary lines of every wall cell.  Make use of the is_clear_path
        helper function.
    */

    // WRITE YOUR CODE HERE
}

void propagate_solo_occupancy(MapLayer<float> &layer, float decay, float growth)
{
    /*
        For every cell in the given layer:

            1) Get the value of each neighbor and apply decay factor
            2) Keep the highest value from step 1
            3) Linearly interpolate from the cell's current value to the value from step 2
               with the growing factor as a coefficient.  Make use of the lerp helper function.
            4) Store the value from step 3 in a temporary layer.
               A float[40][40] will suffice, no need to dynamically allocate or make a new MapLayer.

        After every cell has been processed into the temporary layer, write the temporary layer into
        the given layer;
    */
    
    // WRITE YOUR CODE HERE
}

void propagate_dual_occupancy(MapLayer<float> &layer, float decay, float growth)
{
    /*
        Similar to the solo version, but the values range from -1.0 to 1.0, instead of 0.0 to 1.0

        For every cell in the given layer:

        1) Get the value of each neighbor and apply decay factor
        2) Keep the highest ABSOLUTE value from step 1
        3) Linearly interpolate from the cell's current value to the value from step 2
           with the growing factor as a coefficient.  Make use of the lerp helper function.
        4) Store the value from step 3 in a temporary layer.
           A float[40][40] will suffice, no need to dynamically allocate or make a new MapLayer.

        After every cell has been processed into the temporary layer, write the temporary layer into
        the given layer;
    */

    // WRITE YOUR CODE HERE
}

void normalize_solo_occupancy(MapLayer<float> &layer)
{
    /*
        Determine the maximum value in the given layer, and then divide the value
        for every cell in the layer by that amount.  This will keep the values in the
        range of [0, 1].  Negative values should be left unmodified.
    */

    // WRITE YOUR CODE HERE
}

void normalize_dual_occupancy(MapLayer<float> &layer)
{
    /*
        Similar to the solo version, but you need to track greatest positive value AND 
        the least (furthest from 0) negative value.

        For every cell in the given layer, if the value is currently positive divide it by the
        greatest positive value, or if the value is negative divide it by -1.0 * the least negative value
        (so that it remains a negative number).  This will keep the values in the range of [-1, 1].
    */

    // WRITE YOUR CODE HERE
}

void enemy_field_of_view(MapLayer<float> &layer, float fovAngle, float closeDistance, float occupancyValue, AStarAgent *enemy)
{
    /*
        First, clear out the old values in the map layer by setting any negative value to 0.
        Then, for every cell in the layer that is within the field of view cone, from the
        enemy agent, mark it with the occupancy value.  Take the dot product between the view
        vector and the vector from the agent to the cell, both normalized, and compare the
        cosines directly instead of taking the arccosine to avoid introducing floating-point
        inaccuracy (larger cosine means smaller angle).

        If the tile is close enough to the enemy (less than closeDistance),
        you only check if it's visible to enemy.  Make use of the is_clear_path
        helper function.  Otherwise, you must consider the direction the enemy is facing too.
        This creates a radius around the enemy that the player can be detected within, as well
        as a fov cone.
    */

    // WRITE YOUR CODE HERE
}

bool enemy_find_player(MapLayer<float> &layer, AStarAgent *enemy, Agent *player)
{
    /*
        Check if the player's current tile has a negative value, ie in the fov cone
        or within a detection radius.
    */

    const auto &playerWorldPos = player->get_position();

    const auto playerGridPos = terrain->get_grid_position(playerWorldPos);

    // verify a valid position was returned
    if (terrain->is_valid_grid_position(playerGridPos) == true)
    {
        if (layer.get_value(playerGridPos) < 0.0f)
        {
            return true;
        }
    }

    // player isn't in the detection radius or fov cone, OR somehow off the map
    return false;
}

bool enemy_seek_player(MapLayer<float> &layer, AStarAgent *enemy)
{
    /*
        Attempt to find a cell with the highest nonzero value (normalization may
        not produce exactly 1.0 due to floating point error), and then set it as
        the new target, using enemy->path_to.

        If there are multiple cells with the same highest value, then pick the
        cell closest to the enemy.

        Return whether a target cell was found.
    */

    // WRITE YOUR CODE HERE

    return false; // REPLACE THIS
}
