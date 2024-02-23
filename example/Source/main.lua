import "CoreLibs/object"
import "CoreLibs/graphics"
import "CoreLibs/sprites"

local gfx <const> = playdate.graphics
local screen <const> = playdate.display
local v2d <const> = collision.vector2D
local poly <const> = collision.polygon
local coll <const> = collision


local positions = {
    v2d.new(50,50),
    v2d.new(260,50),
    v2d.new(260,150),
    v2d.new(50,150),
}
local velocities = {
    v2d.new(3,1),
    v2d.new(-2,2),
    v2d.new(1,1),
    v2d.new(3,-2),
}
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

local function render()
    gfx.clear(gfx.kColorWhite)

    for i = 1, #positions do
        gfx.drawCircleAtPoint(positions[i], radius)
    end
    drawV2DListAsPoly(bigPoly)
    -- gfx.drawCircleAtPoint(polyMiddle, polyRadius)

    playdate.drawFPS(380,2)
    gfx.drawText(string.format("%d", #positions), 380, 17)
end

local time = 0
local sum = 0
local count = 0

local colNormal = nil
local depth = 0

function playdate.update()
    time = playdate.getElapsedTime()

    for i=1, #positions do
        if ((positions[i].x < radius) and (velocities[i].x < 0)) or
                ((positions[i].x >= screen.getWidth() - radius) and (velocities[i].x > 0)) then
            velocities[i].x = -velocities[i].x
        end
        if ((positions[i].y < radius) and (velocities[i].y < 0)) or
                ((positions[i].y >= screen.getHeight() - radius) and (velocities[i].y > 0)) then
            velocities[i].y = -velocities[i].y
        end

        positions[i]:addScaled(velocities[i], 1)
    end

    for i=1, #positions do
        for k=i+1, #positions do
            local collides = coll.circleCircle_check(positions[i], radius, positions[k], radius)
            if not collides then goto continue_inner end

            colNormal, depth = coll.circleCircle(positions[i], radius, positions[k], radius)
            positions[i]:addScaled(colNormal, -depth / 2)
            positions[k]:addScaled(colNormal, depth / 2)

            local velDiff = v2d.new(velocities[i].x - velocities[k].x, velocities[i].y - velocities[k].y)
            local relSpeed = colNormal:dotProduct(velDiff)
            velocities[i]:addScaled(colNormal, -relSpeed)
            velocities[k]:addScaled(colNormal, relSpeed)

            ::continue_inner::
        end

        local collides = coll.circleCircle_check(positions[i], radius, polyMiddle, polyRadius)
        if not collides then goto continue end

        colNormal, depth = coll.circlePoly(positions[i], radius, bigPoly)
        if colNormal ~= nil then
            positions[i]:addScaled(colNormal, -depth)

            local dp = colNormal:dotProduct(velocities[i])
            velocities[i]:addScaled(colNormal, -2 * dp)
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

    table.insert(positions, pos)
    local vel = v2d.new(math.random() * 2 + 1, math.random() * 2 + 1)
    table.insert(velocities, vel)
end

function playdate.BButtonUp()
    local count = #positions
    for i=1, count do positions[i]=nil end
    for i=1, count do velocities[i]=nil end
end

---

playdate.display.setRefreshRate(50)

local menu = playdate.getSystemMenu()

local bechmarkItem, _ = menu:addMenuItem("Benchmark", function()
    local count = #positions
    for i=1, count do positions[i]=nil end
    for i=1, count do velocities[i]=nil end

    table.insert(positions, v2d.new(50,50))
    table.insert(positions, v2d.new(260,50))
    table.insert(positions, v2d.new(260,150))
    table.insert(positions, v2d.new(50,150))
    table.insert(positions, v2d.new(20,20))
    table.insert(positions, v2d.new(80,30))
    table.insert(positions, v2d.new(130,40))
    table.insert(positions, v2d.new(200,50))
    table.insert(positions, v2d.new(310,40))
    table.insert(positions, v2d.new(370,30))
    table.insert(positions, v2d.new(370,20))
    table.insert(positions, v2d.new(70,100))
    table.insert(positions, v2d.new(60,220))
    table.insert(positions, v2d.new(280,20))
    table.insert(positions, v2d.new(280,110))
    table.insert(positions, v2d.new(280,210))
    table.insert(positions, v2d.new(100,100))
    table.insert(positions, v2d.new(200,200))
    table.insert(positions, v2d.new(240,200))
    table.insert(positions, v2d.new(160,200))

    table.insert(velocities, v2d.new(3,1))
    table.insert(velocities, v2d.new(-2,2))
    table.insert(velocities, v2d.new(1,1))
    table.insert(velocities, v2d.new(3,-2))
    table.insert(velocities, v2d.new(4,-1))
    table.insert(velocities, v2d.new(-1,-2))
    table.insert(velocities, v2d.new(1,-2))
    table.insert(velocities, v2d.new(-4,2))
    table.insert(velocities, v2d.new(3,1))
    table.insert(velocities, v2d.new(-2,3))
    table.insert(velocities, v2d.new(-1,1))
    table.insert(velocities, v2d.new(2,1))
    table.insert(velocities, v2d.new(0.5,2))
    table.insert(velocities, v2d.new(0.5,-1.5))
    table.insert(velocities, v2d.new(1,0.75))
    table.insert(velocities, v2d.new(1.75,-0.75))
    table.insert(velocities, v2d.new(-3,-0.75))
    table.insert(velocities, v2d.new(2,1))
    table.insert(velocities, v2d.new(-2,2))
    table.insert(velocities, v2d.new(1,2))

    print("--- Starting benchmark...");
    for i=1,3 do
        local benchTime = playdate.getElapsedTime()
        for k=1,100 do
            playdate.update()
        end
        benchTime = playdate.getElapsedTime() - benchTime
        print(string.format("Round %d: 100 frames in %.2fms", i, benchTime * 1000))
    end
    print("--- Benchmark finished")
end)