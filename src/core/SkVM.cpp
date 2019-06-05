/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkVx.h"
#include "src/core/SkOpts.h"
#include "src/core/SkVM.h"
#include <string.h>
#if defined(SK_BUILD_FOR_WIN)
    #include <intrin.h>
#endif

namespace skvm {

    Program::Program(std::vector<Instruction> instructions, int regs)
        : fInstructions(std::move(instructions))
        , fRegs(regs)
    {}

    Program Builder::done() {
        // Basic liveness analysis (and free dead code elimination).
        for (ID id = fProgram.size(); id --> 0; ) {
            Instruction& inst = fProgram[id];

            // All side-effect-only instructions (stores) are live.
            if (inst.op <= Op::store32) {
                inst.life = id;
            }
            // The arguments of a live instruction must live until that instruction.
            if (inst.life != NA) {
                // Notice how we're walking backward, storing the latest instruction in life.
                if (inst.x != NA && fProgram[inst.x].life == NA) { fProgram[inst.x].life = id; }
                if (inst.y != NA && fProgram[inst.y].life == NA) { fProgram[inst.y].life = id; }
                if (inst.z != NA && fProgram[inst.z].life == NA) { fProgram[inst.z].life = id; }
            }
        }

        // We'll need to map each live value to a register.
        std::unordered_map<ID, ID> val_to_reg;

        // Count the registers we've used so far, and track any registers available to reuse.
        ID next_reg = 0;
        std::vector<ID> avail;

        // A schedule of which registers become available as we reach any given instruction.
        std::unordered_map<ID, std::vector<ID>> deaths;

        for (ID val = 0; val < (ID)fProgram.size(); val++) {
            Instruction& inst = fProgram[val];
            if (inst.life == NA) {
                continue;
            }

            // All the values that are no longer needed after this instruction
            // can make their registers available to this and future values.
            const std::vector<ID>& dying = deaths[val];
            avail.insert(avail.end(),
                         dying.begin(), dying.end());

            // Allocate a register if we have to, but prefer to reuse one that's available.
            ID reg;
            if (avail.empty()) {
                reg = next_reg++;
            } else {
                reg = avail.back();
                avail.pop_back();
            }

            // Schedule this value's own death.  When we reach the instruction at inst.life,
            // this value is no longer needed and its register becomes available for reuse.
            deaths[inst.life].push_back(reg);

            val_to_reg[val] = reg;
        }

        // Add a dummy mapping for the N/A sentinel value to "register N/A",
        // so that the lookups don't have to know which arguments are used by which Ops.
        auto lookup_register = [&](ID val) {
            return val == NA ? NA
                             : val_to_reg[val];
        };

        std::vector<Program::Instruction> program;
        for (ID id = 0; id < (ID)fProgram.size(); id++) {
            Instruction& inst = fProgram[id];
            if (inst.life == NA) {
                continue;
            }

            Program::Instruction pinst{
                inst.op,
                lookup_register(id),
                lookup_register(inst.x),
               {lookup_register(inst.y)},
               {lookup_register(inst.z)},
            };
            if (inst.y == NA) { pinst.y.imm = inst.immy; }
            if (inst.z == NA) { pinst.z.imm = inst.immz; }
            program.push_back(pinst);
        }

        return { std::move(program), /*register count = */next_reg };
    }

    // Most instructions produce a value and return it by ID,
    // the value-producing instruction's own index in the program vector.

    ID Builder::push(Op op, ID x, ID y, ID z, int immy, int immz) {
        Instruction inst{op, /*life=*/NA, x, y, z, immy, immz};

        // Basic common subexpression elimination:
        // if we've already seen this exact Instruction, use it instead of creating a new one.
        auto lookup = fIndex.find(inst);
        if (lookup != fIndex.end()) {
            return lookup->second;
        }

        ID id = static_cast<ID>(fProgram.size());
        fProgram.push_back(inst);
        fIndex[inst] = id;
        return id;
    }

    bool Builder::isZero(ID id) const {
        return fProgram[id].op   == Op::splat
            && fProgram[id].immy == 0;
    }

    Arg Builder::arg(int ix) { return {ix}; }

    void Builder::store8 (Arg ptr, I32 val) { (void)this->push(Op::store8 , val.id,NA,NA, ptr.ix); }
    void Builder::store32(Arg ptr, I32 val) { (void)this->push(Op::store32, val.id,NA,NA, ptr.ix); }

    I32 Builder::load8 (Arg ptr) { return {this->push(Op::load8 , NA,NA,NA, ptr.ix) }; }
    I32 Builder::load32(Arg ptr) { return {this->push(Op::load32, NA,NA,NA, ptr.ix) }; }

    // The two splat() functions are just syntax sugar over splatting a 4-byte bit pattern.
    I32 Builder::splat(int   n) { return {this->push(Op::splat, NA,NA,NA, n) }; }
    F32 Builder::splat(float f) {
        int bits;
        memcpy(&bits, &f, 4);
        return {this->push(Op::splat, NA,NA,NA, bits)};
    }

    F32 Builder::add(F32 x, F32 y       ) { return {this->push(Op::add_f32, x.id, y.id)}; }
    F32 Builder::sub(F32 x, F32 y       ) { return {this->push(Op::sub_f32, x.id, y.id)}; }
    F32 Builder::mul(F32 x, F32 y       ) { return {this->push(Op::mul_f32, x.id, y.id)}; }
    F32 Builder::div(F32 x, F32 y       ) { return {this->push(Op::div_f32, x.id, y.id)}; }
    F32 Builder::mad(F32 x, F32 y, F32 z) {
        if (this->isZero(z.id)) {
            return this->mul(x,y);
        }
        return {this->push(Op::mad_f32, x.id, y.id, z.id)};
    }

    I32 Builder::add(I32 x, I32 y) { return {this->push(Op::add_i32, x.id, y.id)}; }
    I32 Builder::sub(I32 x, I32 y) { return {this->push(Op::sub_i32, x.id, y.id)}; }
    I32 Builder::mul(I32 x, I32 y) { return {this->push(Op::mul_i32, x.id, y.id)}; }

    I32 Builder::bit_and(I32 x, I32 y) { return {this->push(Op::bit_and, x.id, y.id)}; }
    I32 Builder::bit_or (I32 x, I32 y) { return {this->push(Op::bit_or , x.id, y.id)}; }
    I32 Builder::bit_xor(I32 x, I32 y) { return {this->push(Op::bit_xor, x.id, y.id)}; }

    I32 Builder::shl(I32 x, int bits) { return {this->push(Op::shl, x.id,NA,NA, bits)}; }
    I32 Builder::shr(I32 x, int bits) { return {this->push(Op::shr, x.id,NA,NA, bits)}; }
    I32 Builder::sra(I32 x, int bits) { return {this->push(Op::sra, x.id,NA,NA, bits)}; }

    I32 Builder::mul_unorm8(I32 x, I32 y) { return {this->push(Op::mul_unorm8, x.id, y.id)}; }

    I32 Builder::extract(I32 x, int mask) {
        SkASSERT(mask != 0);
    #if defined(SK_BUILD_FOR_WIN)
        unsigned long shift;
        _BitScanForward(&shift, mask);
    #else
        const int shift = __builtin_ctz(mask);
    #endif
        if ((unsigned)mask == (~0u << shift)) {
            return this->shr(x, shift);
        }
        return {this->push(Op::extract, x.id,NA,NA, mask, shift)};
    }

    I32 Builder::pack(I32 x, I32 y, int bits) {
        return {this->push(Op::pack, x.id,y.id,NA, 0,bits)};
    }

    F32 Builder::to_f32(I32 x) { return {this->push(Op::to_f32, x.id)}; }
    I32 Builder::to_i32(F32 x) { return {this->push(Op::to_i32, x.id)}; }

    // ~~~~ Program::dump() and co. ~~~~ //

    struct R { ID id; };
    struct Shift { int bits; };
    struct Mask  { int bits; };
    struct Splat { int bits; };

    static void write(SkWStream* o, const char* s) {
        o->writeText(s);
    }

    static void write(SkWStream* o, Arg a) {
        write(o, "arg(");
        o->writeDecAsText(a.ix);
        write(o, ")");
    }
    static void write(SkWStream* o, R r) {
        write(o, "r");
        o->writeDecAsText(r.id);
    }
    static void write(SkWStream* o, Shift s) {
        o->writeDecAsText(s.bits);
    }
    static void write(SkWStream* o, Mask m) {
        o->writeHexAsText(m.bits);
    }
    static void write(SkWStream* o, Splat s) {
        float f;
        memcpy(&f, &s.bits, 4);
        o->writeHexAsText(s.bits);
        write(o, " (");
        o->writeScalarAsText(f);
        write(o, ")");
    }

    template <typename T, typename... Ts>
    static void write(SkWStream* o, T first, Ts... rest) {
        write(o, first);
        write(o, " ");
        write(o, rest...);
    }

    void Program::dump(SkWStream* o) const {
        o->writeDecAsText(fRegs);
        o->writeText(" registers, ");
        o->writeDecAsText(fInstructions.size());
        o->writeText(" instructions:\n");
        for (const Instruction& inst : fInstructions) {
            Op  op = inst.op;
            ID   d = inst.d,
                 x = inst.x;
            auto y = inst.y,
                 z = inst.z;
            switch (op) {
                case Op::store8:  write(o, "store8" , Arg{y.imm}, R{x}); break;
                case Op::store32: write(o, "store32", Arg{y.imm}, R{x}); break;

                case Op::load8:  write(o, R{d}, "= load8" , Arg{y.imm}); break;
                case Op::load32: write(o, R{d}, "= load32", Arg{y.imm}); break;

                case Op::splat:  write(o, R{d}, "= splat", Splat{y.imm}); break;

                case Op::add_f32: write(o, R{d}, "= add_f32", R{x}, R{y.id}           ); break;
                case Op::sub_f32: write(o, R{d}, "= sub_f32", R{x}, R{y.id}           ); break;
                case Op::mul_f32: write(o, R{d}, "= mul_f32", R{x}, R{y.id}           ); break;
                case Op::div_f32: write(o, R{d}, "= div_f32", R{x}, R{y.id}           ); break;
                case Op::mad_f32: write(o, R{d}, "= mad_f32", R{x}, R{y.id}, R{z.id}); break;

                case Op::add_i32: write(o, R{d}, "= add_i32", R{x}, R{y.id}); break;
                case Op::sub_i32: write(o, R{d}, "= sub_i32", R{x}, R{y.id}); break;
                case Op::mul_i32: write(o, R{d}, "= mul_i32", R{x}, R{y.id}); break;

                case Op::bit_and: write(o, R{d}, "= bit_and", R{x}, R{y.id}); break;
                case Op::bit_or : write(o, R{d}, "= bit_or" , R{x}, R{y.id}); break;
                case Op::bit_xor: write(o, R{d}, "= bit_xor", R{x}, R{y.id}); break;

                case Op::shl: write(o, R{d}, "= shl", R{x}, Shift{y.imm}); break;
                case Op::shr: write(o, R{d}, "= shr", R{x}, Shift{y.imm}); break;
                case Op::sra: write(o, R{d}, "= sra", R{x}, Shift{y.imm}); break;

                case Op::mul_unorm8: write(o, R{d}, "= mul_unorm8", R{x}, R{y.id}); break;

                case Op::extract: write(o, R{d}, "= extract", R{x}, Mask{y.imm}); break;
                case Op::pack: write(o, R{d}, "= pack", R{x}, R{y.id}, Shift{z.imm}); break;

                case Op::to_f32: write(o, R{d}, "= to_f32", R{x}); break;
                case Op::to_i32: write(o, R{d}, "= to_i32", R{x}); break;
            }
            write(o, "\n");
        }
    }

    // ~~~~ Program::eval() and co. ~~~~ //

    void Program::eval(int n, void* args[], size_t strides[], int nargs) const {
        SkOpts::eval(fInstructions.data(), (int)fInstructions.size(), fRegs,
                     n, args, strides, nargs);
    }
}
