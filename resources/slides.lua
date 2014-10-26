
gPath = "/skia/trunk/resources/"

function load_file(file)
    package.path = package.path .. ";" .. gPath .. file .. ".lua"
    require(file)
end

load_file("slides_utils")

gSlides = parse_file(io.open("/skia/trunk/resources/slides_content.lua", "r"))

function make_rect(l, t, r, b)
    return { left = l, top = t, right = r, bottom = b }
end

function make_paint(typefacename, stylebits, size, color)
    local paint = Sk.newPaint();
    paint:setAntiAlias(true)
    paint:setSubpixelText(true)
    paint:setTypeface(Sk.newTypeface(typefacename, stylebits))
    paint:setTextSize(size)
    paint:setColor(color)
    return paint
end

function drawSlide(canvas, slide, template)
    template = template.slide   -- need to sniff the slide to know if we're title or slide

    local x = template.margin_x
    local y = template.margin_y

    local scale = 1.25
    for i = 1, #slide do
        local node = slide[i]
        local paint = template[node.indent + 1].paint
        local extra_dy = template[node.indent + 1].extra_dy
        local fm = paint:getFontMetrics()
        local x_offset = -fm.ascent * node.indent

        y = y - fm.ascent * scale
        canvas:drawText(node.text, x + x_offset, y, paint)
        y = y + fm.descent * scale + extra_dy
    end
end

function scale_text_delta(template, delta)
    template = template.slide
    for i = 1, #template do
        local paint = template[i].paint
        paint:setTextSize(paint:getTextSize() + delta)
    end
end

function slide_transition(prev, next, is_forward)
    local rec = {
        proc = function(self, canvas, drawSlideProc)
            if self:isDone() then
                drawSlideProc(canvas)
                return nil
            end
            self.prevDrawable:draw(canvas, self.curr_x, 0)
            self.nextDrawable:draw(canvas, self.curr_x + 640, 0)
            self.curr_x = self.curr_x + self.step_x
            return self
        end
    }
    if is_forward then
        rec.prevDrawable = prev
        rec.nextDrawable = next
        rec.curr_x = 0
        rec.step_x = -15
        rec.isDone = function (self) return self.curr_x <= -640 end
    else
        rec.prevDrawable = next
        rec.nextDrawable = prev
        rec.curr_x = -640
        rec.step_x = 15
        rec.isDone = function (self) return self.curr_x >= 0 end
    end
    return rec
end

function sqr(value) return value * value end

function set_blur(paint, alpha)
    local sigma = sqr(1 - alpha) * 20
--    paint:setImageFilter(Sk.newBlurImageFilter(sigma, sigma))
    paint:setAlpha(alpha)
end

function fade_slide_transition(prev, next, is_forward)
    local rec = {
        paint = Sk.newPaint(),
        prevDrawable = prev,
        nextDrawable = next,
        proc = function(self, canvas, drawSlideProc)
            if self:isDone() then
                drawSlideProc(canvas)
                return nil
            end

            set_blur(self.paint, self.prev_a)
            self.prevDrawable:draw(canvas, self.prev_x, 0, self.paint)

            set_blur(self.paint, self.next_a)
            self.nextDrawable:draw(canvas, self.next_x, 0, self.paint)
            self:step()
            return self
        end
    }
    if is_forward then
        rec.prev_x = 0
        rec.prev_a = 1
        rec.next_x = 640
        rec.next_a = 0
        rec.isDone = function (self) return self.next_x <= 0 end
        rec.step = function (self)
            self.next_x = self.next_x - 20
            self.next_a = (640 - self.next_x) / 640
            self.prev_a = 1 - self.next_a
        end
    else
        rec.prev_x = 0
        rec.prev_a = 1
        rec.next_x = 0
        rec.next_a = 0
        rec.isDone = function (self) return self.prev_x >= 640 end
        rec.step = function (self)
            self.prev_x = self.prev_x + 20
            self.prev_a = (640 - self.prev_x) / 640
            self.next_a = 1 - self.prev_a
        end
    end
    return rec
end

--------------------------------------------------------------------------------------
function make_tmpl(paint, extra_dy)
    return { paint = paint, extra_dy = extra_dy }
end

function SkiaPoint_make_template()
    local title = {
        margin_x = 30,
        margin_y = 100,
    }
    title[1] = make_paint("Arial", 1, 50, { a=1, r=1, g=1, b=1 })
    title[1]:setTextAlign("center")
    title[2] = make_paint("Arial", 1, 25, { a=1, r=.75, g=.75, b=.75 })
    title[2]:setTextAlign("center")

    local slide = {
        margin_x = 20,
        margin_y = 25,
    }
    slide[1] = make_tmpl(make_paint("Arial", 1, 36, { a=1, r=1, g=1, b=1 }), 18)
    slide[2] = make_tmpl(make_paint("Arial", 0, 30, { a=1, r=1, g=1, b=1 }), 0)
    slide[3] = make_tmpl(make_paint("Arial", 0, 24, { a=1, r=.8, g=.8, b=.8 }), 0)

    return {
        title = title,
        slide = slide,
    }
end

gTemplate = SkiaPoint_make_template()

gRedPaint = Sk.newPaint()
gRedPaint:setAntiAlias(true)
gRedPaint:setColor{a=1, r=1, g=0, b=0 }

-- animation.proc is passed the canvas before drawing.
-- The animation.proc returns itself or another animation (which means keep animating)
-- or it returns nil, which stops the animation.
--
local gCurrAnimation

gSlideIndex = 1

function new_drawable_picture(pic)
    return {
        picture = pic,
        width = pic:width(),
        height = pic:height(),
        draw = function (self, canvas, x, y, paint)
            canvas:drawPicture(self.picture, x, y, paint)
        end
    }
end

function new_drawable_image(img)
    return {
        image = img,
        width = img:width(),
        height = img:height(),
        draw = function (self, canvas, x, y, paint)
            canvas:drawImage(self.image, x, y, paint)
        end
    }
end

function new_drawable_slide(slide)
    return {
        slide = slide,
        draw = function (self, canvas, x, y, paint)
            if (nil == paint or ("number" == type(paint) and (1 == paint))) then
                canvas:save()
            else
                canvas:saveLayer(paint)
            end
            canvas:translate(x, y)
            drawSlide(canvas, self.slide, gTemplate)
            canvas:restore()
        end
    }
end

function next_slide()
    local prev = gSlides[gSlideIndex]

    gSlideIndex = gSlideIndex + 1
    if gSlideIndex > #gSlides then
        gSlideIndex = 1
    end

    spawn_transition(prev, gSlides[gSlideIndex], true)
end

function prev_slide()
    local prev = gSlides[gSlideIndex]

    gSlideIndex = gSlideIndex - 1
    if gSlideIndex < 1 then
        gSlideIndex = #gSlides
    end

    spawn_transition(prev, gSlides[gSlideIndex], false)
end

function convert_to_picture_drawable(slide)
    local rec = Sk.newPictureRecorder()
    drawSlide(rec:beginRecording(640, 480), slide, gTemplate)
    return new_drawable_picture(rec:endRecording())
end

function convert_to_image_drawable(slide)
    local surf = Sk.newRasterSurface(640, 480)
    drawSlide(surf:getCanvas(), slide, gTemplate)
    return new_drawable_image(surf:newImageSnapshot())
end

-- gMakeDrawable = convert_to_picture_drawable
gMakeDrawable = new_drawable_slide

function spawn_transition(prevSlide, nextSlide, is_forward)
    local transition
    if is_forward then
        transition = prevSlide.transition
    else
        transition = nextSlide.transition
    end

    if not transition then
        transition = fade_slide_transition
    end

    local prevDrawable = gMakeDrawable(prevSlide)
    local nextDrawable = gMakeDrawable(nextSlide)
    gCurrAnimation = transition(prevDrawable, nextDrawable, is_forward)
end

--------------------------------------------------------------------------------------

function spawn_rotate_animation()
    gCurrAnimation = {
        angle = 0,
        angle_delta = 5,
        pivot_x = 320,
        pivot_y = 240,
        proc = function (self, canvas, drawSlideProc)
            if self.angle >= 360 then
                drawSlideProc(canvas)
                return nil
            end
            canvas:translate(self.pivot_x, self.pivot_y)
            canvas:rotate(self.angle)
            canvas:translate(-self.pivot_x, -self.pivot_y)
            drawSlideProc(canvas)

            self.angle = self.angle + self.angle_delta
            return self
        end
    }
end

function spawn_scale_animation()
    gCurrAnimation = {
        scale = 1,
        scale_delta = .95,
        scale_limit = 0.2,
        pivot_x = 320,
        pivot_y = 240,
        proc = function (self, canvas, drawSlideProc)
            if self.scale < self.scale_limit then
                self.scale = self.scale_limit
                self.scale_delta = 1 / self.scale_delta
            end
            if self.scale > 1 then
                drawSlideProc(canvas)
                return nil
            end
            canvas:translate(self.pivot_x, self.pivot_y)
            canvas:scale(self.scale, self.scale)
            canvas:translate(-self.pivot_x, -self.pivot_y)
            drawSlideProc(canvas)

            self.scale = self.scale * self.scale_delta
            return self
        end
    }
end

local bgPaint = nil

function draw_bg(canvas)
    if not bgPaint then
        bgPaint = Sk.newPaint()
        local grad = Sk.newLinearGradient(  0,   0, { a=1, r=0, g=0, b=.3 },
                                          640, 480, { a=1, r=0, g=0, b=.8 })
        bgPaint:setShader(grad)
    end

    canvas:drawPaint(bgPaint)
end

function onDrawContent(canvas, width, height)
    local matrix = Sk.newMatrix()
    matrix:setRectToRect(make_rect(0, 0, 640, 480), make_rect(0, 0, width, height), "center")
    canvas:concat(matrix)

    draw_bg(canvas)

    local drawSlideProc = function(canvas)
        drawSlide(canvas, gSlides[gSlideIndex], gTemplate)
    end

    if gCurrAnimation then
        gCurrAnimation = gCurrAnimation:proc(canvas, drawSlideProc)
        return true
    else
        drawSlideProc(canvas)
        return false
    end
end

function onClickHandler(x, y)
    return false
end

local keyProcs = {
    n = next_slide,
    p = prev_slide,
    r = spawn_rotate_animation,
    s = spawn_scale_animation,
    ["="] = function () scale_text_delta(gTemplate, 1) end,
    ["-"] = function () scale_text_delta(gTemplate, -1) end,
}

function onCharHandler(uni)
    local proc = keyProcs[uni]
    if proc then
        proc()
        return true
    end
    return false
end
