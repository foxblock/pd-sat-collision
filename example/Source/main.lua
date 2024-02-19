import "CoreLibs/object"
import "CoreLibs/graphics"
import "CoreLibs/sprites"

local gfx <const> = playdate.graphics
local screen <const> = playdate.display
local v2d <const> = collision.vector2D
local poly <const> = collision.polygon
local coll <const> = collision

-- NOTE: Just changing the structure from SOA-style (separate position and vector arrays)
-- to AOS (bundled unit objects) is about 5% slower on its own
local units = {
    { p = v2d.new(50,50), v = v2d.new(3,1) },
    { p = v2d.new(260,50), v = v2d.new(-2,2) },
    { p = v2d.new(260,150), v = v2d.new(1,1) },
    { p = v2d.new(50,150), v = v2d.new(3,-2) },
}
-- local positions = {
--     v2d.new(50,50),
--     v2d.new(260,50),
--     v2d.new(260,150),
--     v2d.new(50,150),
-- }
-- local velocities = {
--     v2d.new(3,1),
--     v2d.new(-2,2),
--     v2d.new(1,1),
--     v2d.new(3,-2),
-- }
local radius = 10
local bigPoly = poly.new(
    120, 120,
    200, 170,
    280, 130,
    256, 100,
    160, 75
)
local polyMiddle, polyRadius = bigPoly:getBoundingCircle()
bigPoly:cacheNormals()


-- list is array of Vector2d structs
-- will always assume poly described by list is closed
local function drawV2DListAsPoly(list)
    for i=1, #list do
        local x1,y1 = list[i]:unpack()
        local x2,y2 = list[(i % #list)+1]:unpack()
        gfx.drawLine(x1,y1,x2,y2)
    end
end

local function compUnit(a, b)
    -- doing the comparison in Lua (a.p.x < b.p.x) is slower by about 10-20%
    return a.p < b.p
end

local checkCount = 0

local function render()
    gfx.clear(gfx.kColorWhite)

    for i = 1, #units do
        gfx.drawCircleAtPoint(units[i].p, radius)
    end
    drawV2DListAsPoly(bigPoly)
    -- gfx.drawCircleAtPoint(polyMiddle, polyRadius)

    playdate.drawFPS(380,2)
    gfx.drawText(string.format("%d", #units), 380, 17)
    gfx.drawText(string.format("%d", checkCount), 370, 32)
end

local time = 0
local sum = 0
local count = 0

local colNormal = nil
local depth = 0

function playdate.update()
    time = playdate.getElapsedTime()

    for i=1, #units do
        if ((units[i].p.x < radius) and (units[i].v.x < 0)) or
                ((units[i].p.x >= screen.getWidth() - radius) and (units[i].v.x > 0)) then
            units[i].v.x = -units[i].v.x
        end
        if ((units[i].p.y < radius) and (units[i].v.y < 0)) or
                ((units[i].p.y >= screen.getHeight() - radius) and (units[i].v.y > 0)) then
            units[i].v.y = -units[i].v.y
        end

        units[i].p:addScaled(units[i].v, 1)
    end

    -- NOTE: Unfortunately the sort in Lua seems to be more expensive than doing the collision checks in C
    -- for 4 balls it's 50-100% slower, for 20 it's about 20% slower
    table.sort(units, compUnit)
    checkCount = 0

    for i=1, #units do
        for k=i+1, #units do
            if units[k].p.x - radius > units[i].p.x + radius then break end

            checkCount = checkCount + 1
            local collides = coll.circleCircle_check(units[i].p, radius, units[k].p, radius)
            if not collides then goto continue_inner end

            colNormal, depth = coll.circleCircle(units[i].p, radius, units[k].p, radius)
            units[i].p:addScaled(colNormal, -depth / 2)
            units[k].p:addScaled(colNormal, depth / 2)

            local velDiff = v2d.new(units[i].v.x - units[k].v.x, units[i].v.y - units[k].v.y)
            local relSpeed = colNormal:dotProduct(velDiff)
            units[i].v:addScaled(colNormal, -relSpeed)
            units[k].v:addScaled(colNormal, relSpeed)

            ::continue_inner::
        end

        local collides = coll.circleCircle_check(units[i].p, radius, polyMiddle, polyRadius)
        if not collides then goto continue end

        colNormal, depth = coll.circlePoly(units[i].p, radius, bigPoly)
        if colNormal ~= nil then
            units[i].p:addScaled(colNormal, -depth)

            local dp = colNormal:dotProduct(units[i].v)
            units[i].v:addScaled(colNormal, -2 * dp)
        end

        ::continue::
    end

    sum = sum + (playdate.getElapsedTime() - time)
    count = count + 1
    if count == 50 then
        print(sum, sum * 100)
        count = 0
        sum = 0
    end

    render()
end

function playdate.AButtonUp()
    local pos = nil
    local collides = true
    while collides do
        pos = v2d.new(math.random(screen.getWidth()), math.random(screen.getHeight()))
        collides = coll.circleCircle_check(pos, radius, polyMiddle, polyRadius)
    end

    local vel = v2d.new(math.random() * 2 + 1, math.random() * 2 + 1)
    table.insert(units, {p = pos, v = vel})
end

function playdate.BButtonUp()
    local count = #units
    for i=1, count do units[i]=nil end
end

---

playdate.display.setRefreshRate(50)