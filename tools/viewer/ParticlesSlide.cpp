/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "ParticlesSlide.h"

#include "SkParticleAffector.h"
#include "SkParticleEffect.h"
#include "SkParticleEmitter.h"
#include "SkParticleSerialization.h"
#include "SkReflected.h"

#include "imgui.h"

using namespace sk_app;

namespace {

static SkScalar kDragSize = 8.0f;
static SkTArray<SkPoint*> gDragPoints;
int gDragIndex = -1;

}

///////////////////////////////////////////////////////////////////////////////

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        SkString* s = (SkString*)data->UserData;
        SkASSERT(data->Buf == s->writable_str());
        SkString tmp(data->Buf, data->BufTextLen);
        s->swap(tmp);
        data->Buf = s->writable_str();
    }
    return 0;
}

#if 0
static ImVec2 map_point(float x, float y, ImVec2 pos, ImVec2 size, float yMin, float yMax) {
    // Turn y into 0 - 1 value
    float yNorm = 1.0f - ((y - yMin) / (yMax - yMin));
    return ImVec2(pos.x + size.x * x, pos.y + size.y * yNorm);
}

static void ImGui_DrawCurve(SkScalar* pts) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Fit our image/canvas to the available width, and scale the height to maintain aspect ratio.
    float canvasWidth = SkTMax(ImGui::GetContentRegionAvailWidth(), 50.0f);
    ImVec2 size = ImVec2(canvasWidth, canvasWidth);
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Background rectangle
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 0, 0, 128));

    // Determine min/max extents
    float yMin = pts[0], yMax = pts[0];
    for (int i = 1; i < 4; ++i) {
        yMin = SkTMin(yMin, pts[i]);
        yMax = SkTMax(yMax, pts[i]);
    }

    // Grow the extents by 10%, at least 1.0f
    float grow = SkTMax((yMax - yMin) * 0.1f, 1.0f);

    yMin -= grow;
    yMax += grow;

    ImVec2 a = map_point(0.0f    , pts[0], pos, size, yMin, yMax),
           b = map_point(1 / 3.0f, pts[1], pos, size, yMin, yMax),
           c = map_point(2 / 3.0f, pts[2], pos, size, yMin, yMax),
           d = map_point(1.0f    , pts[3], pos, size, yMin, yMax);

    drawList->AddBezierCurve(a, b, c, d, IM_COL32(255, 255, 255, 255), 1.0f);

    // Draw markers
    drawList->AddCircle(a, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(b, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(c, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(d, 5.0f, 0xFFFFFFFF);

    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y));
    ImGui::Spacing();
}
#endif

class SkGuiVisitor : public SkFieldVisitor {
public:
    SkGuiVisitor() {
        fTreeStack.push_back(true);
    }

#define IF_OPEN(WIDGET) if (fTreeStack.back()) { WIDGET; }

    void visit(const char* name, float& f, SkField field) override {
        if (fTreeStack.back()) {
            if (field.fFlags & SkField::kAngle_Field) {
                ImGui::SliderAngle(item(name), &f, 0.0f);
            } else {
                ImGui::DragFloat(item(name), &f);
            }
        }
    }
    void visit(const char* name, int& i, SkField) override {
        IF_OPEN(ImGui::DragInt(item(name), &i))
    }
    void visit(const char* name, bool& b, SkField) override {
        IF_OPEN(ImGui::Checkbox(item(name), &b))
    }
    void visit(const char* name, SkString& s, SkField) override {
        if (fTreeStack.back()) {
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
            ImGui::InputText(item(name), s.writable_str(), s.size() + 1, flags, InputTextCallback,
                             &s);
        }
    }

    void visit(const char* name, SkPoint& p, SkField) override {
        if (fTreeStack.back()) {
            ImGui::DragFloat2(item(name), &p.fX);
            gDragPoints.push_back(&p);
        }
    }
    void visit(const char* name, SkColor4f& c, SkField) override {
        IF_OPEN(ImGui::ColorEdit4(item(name), c.vec()))
    }

#undef IF_OPEN

    /*
    void visit(const char* name, SkCurve& c, SkField) override {
        this->enterObject(item(name));
        if (fTreeStack.back()) {
            ImGui::Checkbox("Ranged", &c.fRanged);
            ImGui::DragFloat4("Min", c.fMin);
            ImGui_DrawCurve(c.fMin);
            if (c.fRanged) {
                ImGui::DragFloat4("Max", c.fMax);
                ImGui_DrawCurve(c.fMax);
            }

        }
        this->exitObject();
    }
    */

    void visit(sk_sp<SkReflected>& e, const SkReflected::Type* baseType) override {
        if (fTreeStack.back()) {
            const SkReflected::Type* curType = e ? e->getType() : nullptr;
            if (ImGui::BeginCombo("Type", curType ? curType->fName : "Null")) {
                auto visitType = [curType,&e](const SkReflected::Type* t) {
                    if (ImGui::Selectable(t->fName, curType == t)) {
                        e = t->fFactory();
                    }
                };
                SkReflected::VisitTypes(visitType, baseType);
                ImGui::EndCombo();
            }
        }
    }

    void enterObject(const char* name) override {
        if (fTreeStack.back()) {
            fTreeStack.push_back(ImGui::TreeNodeEx(item(name),
                                                   ImGuiTreeNodeFlags_AllowItemOverlap));
        } else {
            fTreeStack.push_back(false);
        }
    }
    void exitObject() override {
        if (fTreeStack.back()) {
            ImGui::TreePop();
        }
        fTreeStack.pop_back();
    }

    int enterArray(const char* name, int oldCount) override {
        this->enterObject(item(name));
        fArrayCounterStack.push_back(0);
        fArrayEditStack.push_back();

        int count = oldCount;
        if (fTreeStack.back()) {
            ImGui::SameLine();
            if (ImGui::Button("+")) {
                ++count;
            }
        }
        return count;
    }
    ArrayEdit exitArray() override {
        fArrayCounterStack.pop_back();
        auto edit = fArrayEditStack.back();
        fArrayEditStack.pop_back();
        this->exitObject();
        return edit;
    }

private:
    const char* item(const char* name) {
        if (name) {
            return name;
        }

        // We're in an array. Add extra controls and a dynamic label.
        int index = fArrayCounterStack.back()++;
        ArrayEdit& edit(fArrayEditStack.back());
        fScratchLabel = SkStringPrintf("[%d]", index);

        ImGui::PushID(index);

        if (ImGui::Button("X")) {
            edit.fVerb = ArrayEdit::Verb::kRemove;
            edit.fIndex = index;
        }
        ImGui::SameLine();
        if (ImGui::Button("^")) {
            edit.fVerb = ArrayEdit::Verb::kMoveForward;
            edit.fIndex = index;
        }
        ImGui::SameLine();
        if (ImGui::Button("v")) {
            edit.fVerb = ArrayEdit::Verb::kMoveForward;
            edit.fIndex = index + 1;
        }
        ImGui::SameLine();

        ImGui::PopID();

        return fScratchLabel.c_str();
    }

    SkSTArray<16, bool, true> fTreeStack;
    SkSTArray<16, int, true>  fArrayCounterStack;
    SkSTArray<16, ArrayEdit, true> fArrayEditStack;
    SkString fScratchLabel;
};

static sk_sp<SkParticleEffectParams> LoadEffectParams(const char* filename) {
    sk_sp<SkParticleEffectParams> params(new SkParticleEffectParams());
    if (auto fileData = SkData::MakeFromFileName(filename)) {
        skjson::DOM dom(static_cast<const char*>(fileData->data()), fileData->size());
        SkFromJsonVisitor fromJson(dom.root());
        params->visitFields(&fromJson);
    }
    return params;
}

ParticlesSlide::ParticlesSlide() {
    // Register types for serialization
    REGISTER_REFLECTED(SkReflected);
    SkParticleAffector::RegisterAffectorTypes();
    SkParticleEmitter::RegisterEmitterTypes();
    fName = "Particles";
}

void ParticlesSlide::load(SkScalar winWidth, SkScalar winHeight) {
    fEffect.reset(new SkParticleEffect(LoadEffectParams("resources/particles/default.json"),
                                       fRandom));
}

void ParticlesSlide::draw(SkCanvas* canvas) {
    canvas->clear(0);

    gDragPoints.reset();
    if (ImGui::Begin("Particles")) {
        static bool looped = true;
        ImGui::Checkbox("Looped", &looped);
        if (fTimer && ImGui::Button("Play")) {
            fEffect->start(*fTimer, looped);
        }
        static char filename[64] = "resources/particles/default.json";
        ImGui::InputText("Filename", filename, sizeof(filename));
        if (ImGui::Button("Load")) {
            if (auto newParams = LoadEffectParams(filename)) {
                fEffect.reset(new SkParticleEffect(std::move(newParams), fRandom));
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Save")) {
            SkFILEWStream fileStream(filename);
            if (fileStream.isValid()) {
                SkJSONWriter writer(&fileStream, SkJSONWriter::Mode::kPretty);
                SkToJsonVisitor toJson(writer);
                writer.beginObject();
                fEffect->getParams()->visitFields(&toJson);
                writer.endObject();
                writer.flush();
                fileStream.flush();
            } else {
                SkDebugf("Failed to open file\n");
            }
        }

        SkGuiVisitor gui;
        fEffect->getParams()->visitFields(&gui);
    }
    ImGui::End();

    SkPaint dragPaint;
    dragPaint.setColor(SK_ColorLTGRAY);
    dragPaint.setAntiAlias(true);
    SkPaint dragHighlight;
    dragHighlight.setStyle(SkPaint::kStroke_Style);
    dragHighlight.setColor(SK_ColorGREEN);
    dragHighlight.setStrokeWidth(2);
    dragHighlight.setAntiAlias(true);
    for (int i = 0; i < gDragPoints.count(); ++i) {
        canvas->drawCircle(*gDragPoints[i], kDragSize, dragPaint);
        if (gDragIndex == i) {
            canvas->drawCircle(*gDragPoints[i], kDragSize, dragHighlight);
        }
    }
    fEffect->draw(canvas);
}

bool ParticlesSlide::animate(const SkAnimTimer& timer) {
    fTimer = &timer;
    fEffect->update(timer);
    return true;
}

bool ParticlesSlide::onMouse(SkScalar x, SkScalar y, Window::InputState state, uint32_t modifiers) {
    if (gDragIndex == -1) {
        if (state == Window::kDown_InputState) {
            float bestDistance = kDragSize;
            SkPoint mousePt = { x, y };
            for (int i = 0; i < gDragPoints.count(); ++i) {
                float distance = SkPoint::Distance(*gDragPoints[i], mousePt);
                if (distance < bestDistance) {
                    gDragIndex = i;
                    bestDistance = distance;
                }
            }
            return gDragIndex != -1;
        }
    } else {
        // Currently dragging
        SkASSERT(gDragIndex < gDragPoints.count());
        gDragPoints[gDragIndex]->set(x, y);
        if (state == Window::kUp_InputState) {
            gDragIndex = -1;
        }
        return true;
    }
    return false;
}
