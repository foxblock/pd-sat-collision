-- Usage of this module is not recommended for performance reasons
-- Use the C code provided in this repository instead:
-- https://github.com/foxblock/playdate-sat-collision

if collision then return end  -- avoid loading twice the same module
collision = {}  -- create a table to represent the module

local v2d <const> = playdate.geometry.vector2D
local gfx <const> = playdate.graphics

-- writes the left-hand normal of v into target
-- assuming coordinate space where y=0 is top-left and y=+ is bottom-left
local function leftNormalIM(target, v)
    target.x, target.y = v.y, -v.x
end

-- writes the right-hand normal of v into target
-- assuming coordinate space where y=0 is top-left and y=+ is bottom-left
local function rightNormalIM(target, v)
    target.x, target.y = -v.y, v.x
end

local function distanceSquared(pa, pb)
    return (pa.x - pb.x)^2 + (pa.y - pb.y)^2
end

-- writes the normalized direction from pa to pb into target
local function dirNormalized(target, pa, pb)
    local len = math.sqrt((pa.x - pb.x)^2 + (pa.y - pb.y)^2)
    target.x, target.y = (pa.x - pb.x) / len, (pa.y - pb.y) / len
end

-- writes the edge of poly starting with vertex[index] into target
local function polyEdge(target, poly, index)
    local index2 = (index % #poly) + 1
    target.x, target.y = poly[index2].x - poly[index].x, poly[index2].y - poly[index].y
end

-- returns a new vector, which is the middle of polygon p
local function polyMiddle(p)
    local sumX, sumY = 0,0
    for i = 1, #p do
        sumX = sumX + p[i].x
        sumY = sumY + p[i].y
    end

    return v2d.new(sumX / #p, sumY / #p)
end

-- projects poly onto axis
-- returns min and max values. min < max
local function projectPoly(poly, axis)
    local min,max = math.huge, -math.huge
    for i = 1, #poly do
        local val = axis:dotProduct(poly[i])
        if val < min then min = val end
        if val > max then max = val end
    end
    return min,max
end

-- projects circle onto axis
-- returns min and max values. min < max
-- note axis is assumed to be normalized
local function projectCircle(center, radius, axis)
    local min = axis:dotProduct(center - axis * radius)
    local max = axis:dotProduct(center + axis * radius)

    if min > max then
        return max,min
    end
    return min,max
end

-- finds the vertex in poly which is closest to target vector
-- returns the index in the vertex list of poly
local function findClosestVertexIndex(target, poly)
    local distSqr = math.huge
    local result = 0
    for i = 1, #poly do
        local val = (poly[i].x - target.x)^2 + (poly[i].y - target.y)^2
        if val < distSqr then
            distSqr = val
            result = i
        end
    end
    return result
end

-- centerA, centerB are playdate.geometry.vector2D
--
-- return resolveDir, depth -> nil,0 when no collision
-- 
-- resolveDir is collision normal (length 1): A -> B, depth > 0
function collision.circleCircleCollision(centerA, radiusA, centerB, radiusB)
    local distSqr = (centerB - centerA):magnitudeSquared()
    if distSqr >= (radiusA + radiusB) * (radiusA + radiusB) then
        return nil,0
    end

    local resolveDir = (centerB - centerA):normalized()
    return resolveDir, radiusA + radiusB - math.sqrt(distSqr)
end

-- polyA, polyB are list of vector2D defined in CCW order
--
-- return resolveDir, depth -> nil,0 when no collision
-- 
-- resolveDir is collision normal (length 1): A -> B, depth > 0
function collision.polyPolyCollision(polyA, polyB)
    local minA, maxA, minB, maxB
    local depth = math.huge
    local resolveDir = nil
    local invertResult = false

    for i = 1, #polyA do
        local edge = polyA[(i % #polyA) + 1] - polyA[i]
        if edge.x == 0 and edge.y == 0 then
            goto continue
        end
        local axis = edge:leftNormal():normalized()

        minA, maxA = projectPoly(polyA, axis)
        minB, maxB = projectPoly(polyB, axis)

        if maxA < minB or maxB < minA then
            return nil,0
        end

        local axisDepth = math.min(maxA - minB, maxB - minA)
        if axisDepth < depth then
            depth = axisDepth
            resolveDir = axis
            invertResult = maxB - minA < maxA - minB
        end
        ::continue::
    end

    for i = 1, #polyB  do
        local edge = polyB[(i % #polyB) + 1] - polyB[i]
        if edge.x == 0 and edge.y == 0 then
            goto continue
        end
        local axis = edge:leftNormal():normalized()

        minA, maxA = projectPoly(polyA, axis)
        minB, maxB = projectPoly(polyB, axis)

        if maxA < minB or maxB < minA then
            return nil,0
        end

        local axisDepth = math.min(maxA - minB, maxB - minA)
        if axisDepth < depth then
            depth = axisDepth
            resolveDir = axis
            invertResult = maxB - minA < maxA - minB
        end
        ::continue::
    end

    if invertResult then
        return -resolveDir, depth
    end
    return resolveDir, depth
end

-- center is playdate.geometry.vector2D
-- poly is list of vector2D defined in CW order
--
-- return resolveDir, depth -> nil,0 when no collision
-- 
-- resolveDir is collision normal (length 1): A -> B, depth > 0
function collision.circlePolyCollision(center, radius, poly)
    local minA, maxA, minB, maxB
    local depth = math.huge
    local resolveDir = nil
    local invertResult = false
    local edge = v2d.new(0,0)
    local axis = v2d.new(0,0)

    for i = 1, #poly do
        polyEdge(edge, poly, i)
        if edge.x == 0 and edge.y == 0 then
            goto continue
        end
        leftNormalIM(axis,edge)
        axis:normalize()

        minA, maxA = projectCircle(center, radius, axis)
        minB, maxB = projectPoly(poly, axis)

        if maxA < minB or maxB < minA then
            return nil,0
        end

        local axisDepth = math.min(maxA - minB, maxB - minA)
        if axisDepth < depth then
            depth = axisDepth
            resolveDir = v2d.new(axis:unpack())
            invertResult = maxB - minA < maxA - minB
        end
        ::continue::
    end

    local index = findClosestVertexIndex(center, poly)
    dirNormalized(axis, poly[index], center)
    minA, maxA = projectCircle(center, radius, axis)
    minB, maxB = projectPoly(poly, axis)
    if maxA < minB or maxB < minA then
        return nil,0
    end
    local axisDepth = math.min(maxA - minB, maxB - minA)
    if axisDepth < depth then
        depth = axisDepth
        resolveDir = axis
        invertResult = maxB - minA < maxA - minB
    end

    if invertResult then
        return -resolveDir, depth
    end
    return resolveDir, depth
end

-- this assumes the sword is the first two vertices of poly and poly is defined in CCW order
function collision.swordSwingResolution(center, radius, poly)
    local minA, maxB

    local axis = v2d.new(0,0)
    dirNormalized(axis, poly[2], poly[1])
    leftNormalIM(axis, axis)

    minA, _ = projectCircle(center, radius, axis)
    maxB = axis:dotProduct(poly[1])

    return -axis, maxB - minA
end

-- list is array of playdate.geometry.vector2D structs
-- will always assume poly described by list is closed
function collision.drawV2DListAsPoly(list)
    for i=1, #list do
        local x1,y1 = list[i]:unpack()
        local x2,y2 = list[(i % #list)+1]:unpack()
        gfx.drawLine(x1,y1,x2,y2)
    end
end

-- same as drawV2DListAsPoly, but will draw each edge with increasing line width
-- this makes it easier to spot if poly is described in CW or CCW order
function collision.drawV2DListAsPoly2(list)
    local w = gfx.getLineWidth()
    for i=1, #list do
        gfx.setLineWidth(i)
        local x1,y1 = list[i]:unpack()
        local x2,y2 = list[(i % #list)+1]:unpack()
        gfx.drawLine(x1,y1,x2,y2)
    end
    gfx.setLineWidth(w)
end