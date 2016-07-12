function sk_scrape_startcanvas(c, fileName) end
function sk_scrape_endcanvas(c, fileName) end

SkScalarNearlyZero = 1.0 / bit32.lshift(1.0, 12)

function SkScalarNearlyEqual(a, b)
    return math.abs(a,b) <= SkScalarNearlyZero
end

gradients = {}

i = 1

function sk_scrape_accumulate(t)
    local p = t.paint
    if p then
        local s = p:getShader()
        if s then
            local g = s:asAGradient()
            if g then
                gradients[i] = {}

                gradients[i].colorCount = g.colorCount
                gradients[i].type       = g.type
                gradients[i].tile       = g.tile

                numHardStops   = 0
                isEvenlySpaced = true
                for j = 2, g.colorCount, 1 do
                    if not SkScalarNearlyEqual(g.positions[j], j/(g.colorCount-1)) then
                        isEvenlySpaced = false
                    end

                    if SkScalarNearlyEqual(g.positions[j], g.positions[j-1]) then
                        numHardStops = numHardStops + 1
                    end
                end

                gradients[i].isEvenlySpaced = isEvenlySpaced
                gradients[i].numHardStops   = numHardStops;

                gradients[i].positions = {}
                for j = 1, g.colorCount, 1 do
                    gradients[i].positions[j] = g.positions[j]
                end

                i = i + 1
            end
        end
    end
end

function sk_scrape_summarize()
    for k, v in pairs(gradients) do
        local pos = ""
        for j = 1, v.colorCount , 1 do
            pos = pos .. v.positions[j]
            if j ~= v.colorCount then
                pos = pos .. ","
            end
        end

        io.write(string.format("%d %s %s %d %d %s\n",
                                v.colorCount,
                                v.type,
                                v.tile,
                                tonumber(v.isEvenlySpaced and 1 or 0),
                                v.numHardStops,
                                pos))
    end
end

