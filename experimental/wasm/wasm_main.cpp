/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFloatingPoint.h"
#include "SkParsePath.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkString.h"

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

static const int MOVE = 0;
static const int LINE = 1;
static const int QUAD = 2;
static const int CUBIC = 4;
static const int CLOSE = 5;

// =================================================================================
// Creating/Exporting Paths
// =================================================================================

void EMSCRIPTEN_KEEPALIVE SkPathToVerbsArgsArray(SkPath path, emscripten::val /*Array*/ verbs,
                                                 emscripten::val /*Array*/ args) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                verbs.call<void>("push", MOVE);
                args.call<void>("push", pts[0].x());
                args.call<void>("push", pts[0].y());
                break;
            case SkPath::kLine_Verb:
                verbs.call<void>("push", LINE);
                args.call<void>("push", pts[1].x());
                args.call<void>("push", pts[1].y());
                break;
            case SkPath::kQuad_Verb:
                verbs.call<void>("push", QUAD);
                args.call<void>("push", pts[1].x());
                args.call<void>("push", pts[1].y());
                args.call<void>("push", pts[2].x());
                args.call<void>("push", pts[2].y());
                break;
            case SkPath::kConic_Verb:
                printf("unsupported conic verb\n");
                // TODO(kjlubick): Port in the logic from SkParsePath::ToSVGString?
                break;
            case SkPath::kCubic_Verb:
                verbs.call<void>("push", CUBIC);
                args.call<void>("push", pts[1].x());
                args.call<void>("push", pts[1].y());
                args.call<void>("push", pts[2].x());
                args.call<void>("push", pts[2].y());
                args.call<void>("push", pts[3].x());
                args.call<void>("push", pts[3].y());
                break;
            case SkPath::kClose_Verb:
                verbs.call<void>("push", CLOSE);
                break;
            case SkPath::kDone_Verb:
                break;
        }
    }
}

emscripten::val JSArray = emscripten::val::global("Array");

emscripten::val EMSCRIPTEN_KEEPALIVE SkPathToCmdArray(SkPath path) {
    val cmds = JSArray.new_();

    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
        val cmd = JSArray.new_();
        switch (verb) {
            case SkPath::kMove_Verb:
                cmd.call<void>("push", MOVE);
                cmd.call<void>("push", pts[0].x());
                cmd.call<void>("push", pts[0].y());
                break;
            case SkPath::kLine_Verb:
                cmd.call<void>("push", LINE);
                cmd.call<void>("push", pts[1].x());
                cmd.call<void>("push", pts[1].y());
                break;
            case SkPath::kQuad_Verb:
                cmd.call<void>("push", QUAD);
                cmd.call<void>("push", pts[1].x());
                cmd.call<void>("push", pts[1].y());
                cmd.call<void>("push", pts[2].x());
                cmd.call<void>("push", pts[2].y());
                break;
            case SkPath::kConic_Verb:
                printf("unsupported conic verb\n");
                // TODO(kjlubick): Port in the logic from SkParsePath::ToSVGString?
                break;
            case SkPath::kCubic_Verb:
                cmd.call<void>("push", CUBIC);
                cmd.call<void>("push", pts[1].x());
                cmd.call<void>("push", pts[1].y());
                cmd.call<void>("push", pts[2].x());
                cmd.call<void>("push", pts[2].y());
                cmd.call<void>("push", pts[3].x());
                cmd.call<void>("push", pts[3].y());
                break;
            case SkPath::kClose_Verb:
                cmd.call<void>("push", CLOSE);
                break;
            case SkPath::kDone_Verb:
                break;
        }
        cmds.call<void>("push", cmd);
    }
    return cmds;
}

// This type signature is a mess, but it's necessary. See, we can't use "bind" (EMSCRIPTEN_BINDINGS)
// and pointers to primitive types (Only bound types like SkPoint). We could if we used
// cwrap (see https://becominghuman.ai/passing-and-returning-webassembly-array-parameters-a0f572c65d97)
// but that requires us to stick to C code and, AFAIK, doesn't allow us to return nice things like
// SkPath or SkOpBuilder.
//
// So, basically, if we are using C++ and EMSCRIPTEN_BINDINGS, we can't have primative pointers
// in our function type signatures. (this gives an error message like "Cannot call foo due to unbound
// types Pi, Pf").  But, we can just pretend they are numbers and cast them to be pointers and
// the compiler is happy.
SkPath EMSCRIPTEN_KEEPALIVE SkPathFromVerbsArgsTyped(int /* uint8_t* */  vptr, int numVerbs,
                                                     int /* float* */aptr, int numArgs) {
    auto verbs = reinterpret_cast<uint8_t*>(vptr);
    auto args = reinterpret_cast<float*>(aptr);
    SkPath path;
    int argsIndex = 0;
    float x1, y1, x2, y2, x3, y3;

    // if there are not enough arguments, bail with the path we've constructed so far.
    #define CHECK_NUM_ARGS(n) \
        if ((argsIndex + n) > numArgs) { \
            SkDebugf("Not enough args to match the verbs. Saw %d args\n", numArgs); \
            return path; \
        }

    for(int i = 0; i < numVerbs; i++){
         switch (verbs[i]) {
            case MOVE:
                CHECK_NUM_ARGS(2);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                path.moveTo(x1, y1);
                break;
            case LINE:
                CHECK_NUM_ARGS(2);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                path.lineTo(x1, y1);
                break;
            case QUAD:
                CHECK_NUM_ARGS(4);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                x2 = args[argsIndex++], y2 = args[argsIndex++];
                path.quadTo(x1, y1, x2, y2);
                break;
            case CUBIC:
                CHECK_NUM_ARGS(6);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                x2 = args[argsIndex++], y2 = args[argsIndex++];
                x3 = args[argsIndex++], y3 = args[argsIndex++];
                path.cubicTo(x1, y1, x2, y2, x3, y3);
                break;
            case CLOSE:
                path.close();
                break;
            default:
                SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verbs[i]);
                return path;
        }
    }

    #undef CHECK_NUM_ARGS

    return path;
}

// See above comment for rational of pointer mess
SkPath EMSCRIPTEN_KEEPALIVE SkPathFromCmdTyped(int /* float* */cptr, int numCmds) {
    auto cmds = reinterpret_cast<float*>(cptr);
    SkPath path;
    float x1, y1, x2, y2, x3, y3;

    // if there are not enough arguments, bail with the path we've constructed so far.
    #define CHECK_NUM_ARGS(n) \
        if ((i + n) > numCmds) { \
            SkDebugf("Not enough args to match the verbs. Saw %d commands\n", numCmds); \
            return path; \
        }

    for(int i = 0; i < numCmds;){
         switch (sk_float_floor2int(cmds[i++])) {
            case MOVE:
                CHECK_NUM_ARGS(2);
                x1 = cmds[i++], y1 = cmds[i++];
                path.moveTo(x1, y1);
                break;
            case LINE:
                CHECK_NUM_ARGS(2);
                x1 = cmds[i++], y1 = cmds[i++];
                path.lineTo(x1, y1);
                break;
            case QUAD:
                CHECK_NUM_ARGS(4);
                x1 = cmds[i++], y1 = cmds[i++];
                x2 = cmds[i++], y2 = cmds[i++];
                path.quadTo(x1, y1, x2, y2);
                break;
            case CUBIC:
                CHECK_NUM_ARGS(6);
                x1 = cmds[i++], y1 = cmds[i++];
                x2 = cmds[i++], y2 = cmds[i++];
                x3 = cmds[i++], y3 = cmds[i++];
                path.cubicTo(x1, y1, x2, y2, x3, y3);
                break;
            case CLOSE:
                path.close();
                break;
            default:
                SkDebugf("  path: UNKNOWN command %f, aborting dump...\n", cmds[i-1]);
                return path;
        }
    }

    #undef CHECK_NUM_ARGS

    return path;
}

//========================================================================================
// SVG THINGS
//========================================================================================

val EMSCRIPTEN_KEEPALIVE ToSVGString(SkPath path) {
    SkString s;
    SkParsePath::ToSVGString(path, &s);
    // Wrapping it in val automatically turns it into a JS string.
    // Not too sure on performance implications, but is is simpler than
    // returning a raw pointer to const char * and then using
    // Pointer_stringify() on the calling side.
    return val(s.c_str());
}


SkPath EMSCRIPTEN_KEEPALIVE FromSVGString(std::string str) {
    SkPath path;
    SkParsePath::FromSVGString(str.c_str(), &path);
    return path;
}

//========================================================================================
// PATHOP THINGS
//========================================================================================

SkPath EMSCRIPTEN_KEEPALIVE SimplifyPath(SkPath path) {
    SkPath simple;
    Simplify(path, &simple);
    return simple;
}

SkPath EMSCRIPTEN_KEEPALIVE ApplyPathOp(SkPath pathOne, SkPath pathTwo, SkPathOp op) {
    SkPath path;
    Op(pathOne, pathTwo, op, &path);
    return path;
}

SkPath EMSCRIPTEN_KEEPALIVE ResolveBuilder(SkOpBuilder builder) {
    SkPath path;
    builder.resolve(&path);
    return path;
}

//========================================================================================
// Canvas THINGS
//========================================================================================

emscripten::val EMSCRIPTEN_KEEPALIVE ToPath2D(SkPath path, val/* Path2D&*/ retVal) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                retVal.call<void>("moveTo", pts[0].x(), pts[0].y());
                break;
            case SkPath::kLine_Verb:
                retVal.call<void>("lineTo", pts[1].x(), pts[1].y());
                break;
            case SkPath::kQuad_Verb:
                retVal.call<void>("quadraticCurveTo", pts[1].x(), pts[1].y(), pts[2].x(), pts[2].y());
                break;
            case SkPath::kConic_Verb:
                printf("unsupported conic verb\n");
                // TODO(kjlubick): Port in the logic from SkParsePath::ToSVGString?
                break;
            case SkPath::kCubic_Verb:
                retVal.call<void>("bezierCurveTo", pts[1].x(), pts[1].y(), pts[2].x(), pts[2].y(),
                                                   pts[3].x(), pts[3].y());
                break;
            case SkPath::kClose_Verb:
                retVal.call<void>("closePath");
                break;
            case SkPath::kDone_Verb:
                break;
        }
    }
    return retVal;
}

// Binds the classes to the JS
EMSCRIPTEN_BINDINGS(skia) {
    class_<SkPath>("SkPath")
        .constructor<>()

        .function("moveTo",
            select_overload<void(SkScalar, SkScalar)>(&SkPath::moveTo))
        .function("lineTo",
            select_overload<void(SkScalar, SkScalar)>(&SkPath::lineTo))
        .function("quadTo",
            select_overload<void(SkScalar, SkScalar, SkScalar, SkScalar)>(&SkPath::quadTo))
        .function("cubicTo",
            select_overload<void(SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar)>(&SkPath::cubicTo))
        .function("close", &SkPath::close);
        // Uncomment below for debugging.
        //.function("dump", select_overload<void() const>(&SkPath::dump));

    class_<SkOpBuilder>("SkOpBuilder")
        .constructor<>()

        .function("add", &SkOpBuilder::add);

    // Without this, module._ToPath2D (yes with an underscore)
    // would be exposed, but be unable to correctly handle the SkPath type.
    function("ToPath2D", &ToPath2D);
    function("ToSVGString", &ToSVGString);
    function("FromSVGString", &FromSVGString);

    function("SkPathToVerbsArgsArray", &SkPathToVerbsArgsArray);
    function("SkPathFromVerbsArgsTyped", &SkPathFromVerbsArgsTyped);

    function("SkPathFromCmdTyped", &SkPathFromCmdTyped);
    function("SkPathToCmdArray", &SkPathToCmdArray);

    function("SimplifyPath", &SimplifyPath);
    function("ApplyPathOp", &ApplyPathOp);
    function("ResolveBuilder", &ResolveBuilder);

    enum_<SkPathOp>("PathOp")
        .value("DIFFERENCE",         SkPathOp::kDifference_SkPathOp)
        .value("INTERSECT",          SkPathOp::kIntersect_SkPathOp)
        .value("UNION",              SkPathOp::kUnion_SkPathOp)
        .value("XOR",                SkPathOp::kXOR_SkPathOp)
        .value("REVERSE_DIFFERENCE", SkPathOp::kReverseDifference_SkPathOp);

    constant("MOVE_VERB",  MOVE);
    constant("LINE_VERB",  LINE);
    constant("QUAD_VERB",  QUAD);
    constant("CUBIC_VERB", CUBIC);
    constant("CLOSE_VERB", CLOSE);
}
