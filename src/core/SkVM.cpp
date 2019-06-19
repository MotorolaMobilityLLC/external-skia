/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/private/SkSpinlock.h"
#include "include/private/SkThreadID.h"
#include "include/private/SkVx.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOpts.h"
#include "src/core/SkVM.h"
#include <string.h>
#if defined(SKVM_JIT)
    #define XBYAK_NO_OP_NAMES
    #include "xbyak/xbyak.h"
#endif

namespace skvm {

    Program::~Program() = default;
    Program::Program(Program&&) = default;
    Program& Program::operator=(Program&&) = default;

    Program::Program(std::vector<Instruction> instructions, int regs, int loop)
        : fInstructions(std::move(instructions))
        , fRegs(regs)
        , fLoop(loop) {}

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

        // Look to see if there are any instructions that can be hoisted outside the program's loop.
        for (ID id = 0; id < (ID)fProgram.size(); id++) {
            Instruction& inst = fProgram[id];

            // Loads and stores cannot be hoisted out of the loop.
            if (inst.op <= Op::load32) {
                inst.hoist = false;
            }

            // If any of an instruction's arguments can't be hoisted, it can't be hoisted itself.
            if (inst.hoist) {
                if (inst.x != NA) { inst.hoist &= fProgram[inst.x].hoist; }
                if (inst.y != NA) { inst.hoist &= fProgram[inst.y].hoist; }
                if (inst.z != NA) { inst.hoist &= fProgram[inst.z].hoist; }
            }
        }

        // We'll need to map each live value to a register.
        std::unordered_map<ID, ID> val_to_reg;

        // Count the registers we've used so far.
        ID next_reg = 0;

        // Our first pass of register assignment assigns hoisted values to eternal registers.
        for (ID val = 0; val < (ID)fProgram.size(); val++) {
            Instruction& inst = fProgram[val];
            if (inst.life == NA || !inst.hoist) {
                continue;
            }

            // Hoisted values are needed forever, so they each get their own register.
            val_to_reg[val] = next_reg++;
        }

        // Now we'll assign registers to values that can't be hoisted out of the loop.  These
        // values have finite liftimes, so we track pre-owned registers that have become available
        // and a schedule of which registers become available as we reach a given instruction.
        std::vector<ID>                         avail;
        std::unordered_map<ID, std::vector<ID>> deaths;

        for (ID val = 0; val < (ID)fProgram.size(); val++) {
            Instruction& inst = fProgram[val];
            if (inst.life == NA || inst.hoist) {
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

        // Finally translate Builder::Instructions to Program::Instructions by mapping values to
        // registers.  This will be two passes again, first outside the loop, then inside.

        // The loop begins at the loop'th Instruction.
        int loop = 0;
        std::vector<Program::Instruction> program;

        auto push_instruction = [&](ID id, const Builder::Instruction& inst) {
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
        };

        for (ID id = 0; id < (ID)fProgram.size(); id++) {
            Instruction& inst = fProgram[id];
            if (inst.life == NA || !inst.hoist) {
                continue;
            }

            push_instruction(id, inst);
            loop++;
        }
        for (ID id = 0; id < (ID)fProgram.size(); id++) {
            Instruction& inst = fProgram[id];
            if (inst.life == NA || inst.hoist) {
                continue;
            }

            push_instruction(id, inst);
        }

        return { std::move(program), /*register count = */next_reg, loop };
    }

    // Most instructions produce a value and return it by ID,
    // the value-producing instruction's own index in the program vector.

    ID Builder::push(Op op, ID x, ID y, ID z, int immy, int immz) {
        Instruction inst{op, /*hoist=*/true, /*life=*/NA, x, y, z, immy, immz};

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

    I32 Builder::sub_16x2(I32 x, I32 y) { return {this->push(Op::sub_i16x2, x.id, y.id)}; }
    I32 Builder::mul_16x2(I32 x, I32 y) { return {this->push(Op::mul_i16x2, x.id, y.id)}; }
    I32 Builder::shr_16x2(I32 x, int bits) { return {this->push(Op::shr_i16x2, x.id,NA,NA, bits)}; }

    I32 Builder::bit_and(I32 x, I32 y) { return {this->push(Op::bit_and, x.id, y.id)}; }
    I32 Builder::bit_or (I32 x, I32 y) { return {this->push(Op::bit_or , x.id, y.id)}; }
    I32 Builder::bit_xor(I32 x, I32 y) { return {this->push(Op::bit_xor, x.id, y.id)}; }

    I32 Builder::shl(I32 x, int bits) { return {this->push(Op::shl, x.id,NA,NA, bits)}; }
    I32 Builder::shr(I32 x, int bits) { return {this->push(Op::shr, x.id,NA,NA, bits)}; }
    I32 Builder::sra(I32 x, int bits) { return {this->push(Op::sra, x.id,NA,NA, bits)}; }

    I32 Builder::extract(I32 x, int bits, I32 z) {
        return {this->push(Op::extract, x.id,NA,z.id, bits,0)};
    }

    I32 Builder::pack(I32 x, I32 y, int bits) {
        return {this->push(Op::pack, x.id,y.id,NA, 0,bits)};
    }

    I32 Builder::bytes(I32 x, int control) {
        return {this->push(Op::bytes, x.id,NA,NA, control)};
    }

    F32 Builder::to_f32(I32 x) { return {this->push(Op::to_f32, x.id)}; }
    I32 Builder::to_i32(F32 x) { return {this->push(Op::to_i32, x.id)}; }

    // ~~~~ Program::dump() and co. ~~~~ //

    struct V { ID id; };
    struct R { ID id; };
    struct Shift { int bits; };
    struct Splat { int bits; };
    struct Hex   { int bits; };

    static void write(SkWStream* o, const char* s) {
        o->writeText(s);
    }

    static void write(SkWStream* o, Arg a) {
        write(o, "arg(");
        o->writeDecAsText(a.ix);
        write(o, ")");
    }
    static void write(SkWStream* o, V v) {
        write(o, "v");
        o->writeDecAsText(v.id);
    }
    static void write(SkWStream* o, R r) {
        write(o, "r");
        o->writeDecAsText(r.id);
    }
    static void write(SkWStream* o, Shift s) {
        o->writeDecAsText(s.bits);
    }
    static void write(SkWStream* o, Splat s) {
        float f;
        memcpy(&f, &s.bits, 4);
        o->writeHexAsText(s.bits);
        write(o, " (");
        o->writeScalarAsText(f);
        write(o, ")");
    }
    static void write(SkWStream* o, Hex h) {
        o->writeHexAsText(h.bits);
    }

    template <typename T, typename... Ts>
    static void write(SkWStream* o, T first, Ts... rest) {
        write(o, first);
        write(o, " ");
        write(o, rest...);
    }

    void Builder::dump(SkWStream* o) const {
        o->writeDecAsText(fProgram.size());
        o->writeText(" values:\n");
        for (ID id = 0; id < (ID)fProgram.size(); id++) {
            const Instruction& inst = fProgram[id];
            Op  op = inst.op;
            ID   x = inst.x,
                 y = inst.y,
                 z = inst.z;
            int immy = inst.immy,
                immz = inst.immz;
            write(o, inst.life == NA ? "☠ " :
                     inst.hoist      ? "⤴ " : "  ");
            switch (op) {
                case Op::store8:  write(o, "store8" , Arg{immy}, V{x}); break;
                case Op::store32: write(o, "store32", Arg{immy}, V{x}); break;

                case Op::load8:  write(o, V{id}, "= load8" , Arg{immy}); break;
                case Op::load32: write(o, V{id}, "= load32", Arg{immy}); break;

                case Op::splat:  write(o, V{id}, "= splat", Splat{immy}); break;

                case Op::add_f32: write(o, V{id}, "= add_f32", V{x}, V{y}      ); break;
                case Op::sub_f32: write(o, V{id}, "= sub_f32", V{x}, V{y}      ); break;
                case Op::mul_f32: write(o, V{id}, "= mul_f32", V{x}, V{y}      ); break;
                case Op::div_f32: write(o, V{id}, "= div_f32", V{x}, V{y}      ); break;
                case Op::mad_f32: write(o, V{id}, "= mad_f32", V{x}, V{y}, V{z}); break;

                case Op::add_i32: write(o, V{id}, "= add_i32", V{x}, V{y}); break;
                case Op::sub_i32: write(o, V{id}, "= sub_i32", V{x}, V{y}); break;
                case Op::mul_i32: write(o, V{id}, "= mul_i32", V{x}, V{y}); break;

                case Op::sub_i16x2: write(o, V{id}, "= sub_i16x2", V{x}, V{y}); break;
                case Op::mul_i16x2: write(o, V{id}, "= mul_i16x2", V{x}, V{y}); break;
                case Op::shr_i16x2: write(o, V{id}, "= shr_i16x2", V{x}, Shift{immy}); break;

                case Op::bit_and: write(o, V{id}, "= bit_and", V{x}, V{y}); break;
                case Op::bit_or : write(o, V{id}, "= bit_or" , V{x}, V{y}); break;
                case Op::bit_xor: write(o, V{id}, "= bit_xor", V{x}, V{y}); break;

                case Op::shl: write(o, V{id}, "= shl", V{x}, Shift{immy}); break;
                case Op::shr: write(o, V{id}, "= shr", V{x}, Shift{immy}); break;
                case Op::sra: write(o, V{id}, "= sra", V{x}, Shift{immy}); break;

                case Op::extract: write(o, V{id}, "= extract", V{x}, Shift{immy}, V{z}); break;
                case Op::pack:    write(o, V{id}, "= pack",    V{x}, V{y}, Shift{immz}); break;

                case Op::bytes:   write(o, V{id}, "= bytes", V{x}, Hex{immy}); break;

                case Op::to_f32: write(o, V{id}, "= to_f32", V{x}); break;
                case Op::to_i32: write(o, V{id}, "= to_i32", V{x}); break;
            }

            write(o, "\n");
        }
    }

    void Program::dump(SkWStream* o) const {
        o->writeDecAsText(fRegs);
        o->writeText(" registers, ");
        o->writeDecAsText(fInstructions.size());
        o->writeText(" instructions:\n");
        for (int i = 0; i < (int)fInstructions.size(); i++) {
            if (i == fLoop) {
                write(o, "loop:\n");
            }
            const Instruction& inst = fInstructions[i];
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

                case Op::sub_i16x2: write(o, R{d}, "= sub_i16x2", R{x}, R{y.id}); break;
                case Op::mul_i16x2: write(o, R{d}, "= mul_i16x2", R{x}, R{y.id}); break;
                case Op::shr_i16x2: write(o, R{d}, "= shr_i16x2", R{x}, Shift{y.imm}); break;

                case Op::bit_and: write(o, R{d}, "= bit_and", R{x}, R{y.id}); break;
                case Op::bit_or : write(o, R{d}, "= bit_or" , R{x}, R{y.id}); break;
                case Op::bit_xor: write(o, R{d}, "= bit_xor", R{x}, R{y.id}); break;

                case Op::shl: write(o, R{d}, "= shl", R{x}, Shift{y.imm}); break;
                case Op::shr: write(o, R{d}, "= shr", R{x}, Shift{y.imm}); break;
                case Op::sra: write(o, R{d}, "= sra", R{x}, Shift{y.imm}); break;

                case Op::extract: write(o, R{d}, "= extract", R{x}, Shift{y.imm}, R{z.id}); break;
                case Op::pack:    write(o, R{d}, "= pack",    R{x}, R{y.id}, Shift{z.imm}); break;

                case Op::bytes: write(o, R{d}, "= bytes", R{x}, Hex{y.imm}); break;

                case Op::to_f32: write(o, R{d}, "= to_f32", R{x}); break;
                case Op::to_i32: write(o, R{d}, "= to_i32", R{x}); break;
            }
            write(o, "\n");
        }
    }

    // ~~~~ Program::eval() and co. ~~~~ //

#if defined(SKVM_JIT)
    // Handy references for x86-64 instruction encoding:
    // https://wiki.osdev.org/X86-64_Instruction_Encoding
    // https://www-user.tu-chemnitz.de/~heha/viewchm.php/hs/x86.chm/x64.htm

    // Order matters... GP64, XMM, YMM values match 4-bit register encoding for each.
    enum class GP64 {
        rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi,
        r8 , r9,  r10, r11, r12, r13, r14, r15,
    };
    enum class XMM {
        xmm0, xmm1, xmm2 , xmm3 , xmm4 , xmm5 , xmm6 , xmm7 ,
        xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,
    };
    enum class YMM {
        ymm0, ymm1, ymm2 , ymm3 , ymm4 , ymm5 , ymm6 , ymm7 ,
        ymm8, ymm9, ymm10, ymm11, ymm12, ymm13, ymm14, ymm15,
    };

    // Encodes the arguments of an opcode.
    struct ModRM {
        uint8_t bits;

        enum Mod { Indirect, OneByteImm, FourByteImm, Direct };

        ModRM(Mod mod, int reg, int rm) {
            bits = (int)mod  << 6
                 | (reg & 7) << 3
                 | (rm  & 7) << 0;
        }
    };

    // Scale-Index-Base encoding for memory addressing, base + (index * scale).
    struct SIB {
        uint8_t bits;

        enum Scale { One, Two, Four, Eight };

        SIB(Scale scale, int index, int base) {
            bits = (int)scale  << 6
                 | (index & 7) << 3
                 | (base  & 7) << 0;
        }
    };

    // The REX prefix is used to extend most old 32-bit instructions to 64-bit.
    struct REX {
        uint8_t bits;

        // If W is set, the operand size is 64-bit, otherwise the default, usually 32.
        // R,X,B are the top bit for ModRM's reg and SIB's index and base respectively.
        // These will be set if you use any of the second-row registers (r8, xmm8, etc).

        REX(bool W, bool R, bool X, bool B) {
            bits = 0b01000000   // Fixed 0100 for top four bits.
                 | (W << 3)
                 | (R << 2)
                 | (X << 1)
                 | (B << 0);
        }
    };

    // The VEX prefix extends SSE operations to AVX.  Used generally, even with XMM.
    struct VEX {
        uint8_t bits[3];
        // TODO
    };

    Assembler::Assembler() : X{new Xbyak::CodeGenerator} {}
    Assembler::~Assembler() = default;

    void*  Assembler::code() const { return (void*)X->getCode(); }
    size_t Assembler::size() const { return        X->getSize(); }

    void Assembler::byte(uint8_t b) { X->db(b); }

    template <typename... Rest>
    void Assembler::byte(uint8_t first, Rest... rest) {
        this->byte(first);
        this->byte(rest...);
    }

    void Assembler::nop() { this->byte(0x90); }
    void Assembler::align(int mod) {
        while (this->size() % mod) {
            this->nop();
        }
    }

    void Assembler::vzeroupper() { this->byte(0xc5, 0xf8, 0x77); }
    void Assembler::ret() { this->byte(0xc3); }

    static bool can_jit(int regs, int nargs) {
        return true
            && SkCpu::Supports(SkCpu::HSW)   // TODO: SSE4.1 target?
            && regs  <= 15   // All 16 ymm registers, reserving one for us as tmp.
            && nargs <=  5;  // We can increase this if we push/pop GP registers.
    }

    // Returns stride of the JIT, currently always 8.
    static int jit(Assembler& a,
                   const std::vector<Program::Instruction>& instructions,
                   int regs, int loop, size_t strides[], int nargs) {
        SkASSERT(can_jit(regs,nargs));

        Xbyak::CodeGenerator& X = *a.X;

        static constexpr int K = 8;

    #if defined(SK_BUILD_FOR_WIN)
        // TODO  Windows ABI?
    #else
        // These registers are used to pass the first 6 arguments,
        // so if we stick to these we need not push, pop, spill, or move anything around.
        Xbyak::Reg N = X.rdi,
               arg[] = { X.rsi, X.rdx, X.rcx, X.r8, X.r9 };

        // All 16 ymm registers are available as scratch, keeping 15 as a temporary for us.
        Xbyak::Ymm r[] = {
            X.ymm0, X.ymm1, X.ymm2 , X.ymm3 , X.ymm4 , X.ymm5 , X.ymm6 , X.ymm7,
            X.ymm8, X.ymm9, X.ymm10, X.ymm11, X.ymm12, X.ymm13, X.ymm14,
        }, tmp = X.ymm15;
    #endif

        // Label / N-byte values we need to write after ret.
        struct Data4  { Xbyak::Label label; int bits   ; };
        struct Data32 { Xbyak::Label label; int bits[8]; };
        std::vector<Data4 > data4;
        std::vector<Data32> data32;

        // Map from our bytes() control y.imm to index in data32;
        // no need to splat out duplicate bytes for the same control.
        std::unordered_map<int, int> vpshufb_masks;

        for (int i = 0; i < (int)instructions.size(); i++) {
            if (i == loop) {
                X.L("loop");
            }
            const Program::Instruction& inst = instructions[i];
            Op  op = inst.op;

            ID   d = inst.d,
                 x = inst.x;
            auto y = inst.y,
                 z = inst.z;
            switch (op) {
                // Ops producing multiple AVX instructions should always
                // use tmp as the result of all but the final instruction
                // to avoid any possible dst/arg aliasing.  You don't want
                // to overwrite your arguments before you're done using them!

                case Op::store8:
                    if (SkCpu::Supports(SkCpu::SKX)) {
                        // One stop shop!  Pack 8x I32 -> U8 and store to ptr.
                        X.vpmovusdb(X.ptr[arg[y.imm]], r[x]);
                    } else {
                        X.vpackusdw(tmp, r[x], r[x]); // pack 32-bit -> 16-bit
                        X.vpermq   (tmp, tmp, 0xd8);  // u64 tmp[0,1,2,3] = tmp[0,2,1,3]
                        X.vpackuswb(tmp, tmp, tmp);   // pack 16-bit -> 8-bit
                        X.vmovq(X.ptr[arg[y.imm]],         // store low 8 bytes
                                Xbyak::Xmm{tmp.getIdx()}); // (arg must be an xmm)
                    }
                    break;

                case Op::store32: X.vmovups(X.ptr[arg[y.imm]], r[x]); break;

                case Op::load8:  X.vpmovzxbd(r[d], X.ptr[arg[y.imm]]); break;
                case Op::load32: X.vmovups  (r[d], X.ptr[arg[y.imm]]); break;

                case Op::splat: data4.push_back(Data4{Xbyak::Label(), y.imm});
                                X.vbroadcastss(r[d], X.ptr[X.rip + data4.back().label]);
                                break;

                case Op::add_f32: X.vaddps(r[d], r[x], r[y.id]); break;
                case Op::sub_f32: X.vsubps(r[d], r[x], r[y.id]); break;
                case Op::mul_f32: X.vmulps(r[d], r[x], r[y.id]); break;
                case Op::div_f32: X.vdivps(r[d], r[x], r[y.id]); break;
                case Op::mad_f32:
                    if (d == x   ) { X.vfmadd132ps(r[x   ], r[z.id], r[y.id]); } else
                    if (d == y.id) { X.vfmadd213ps(r[y.id], r[x   ], r[z.id]); } else
                    if (d == z.id) { X.vfmadd231ps(r[z.id], r[x   ], r[y.id]); } else
                                   { X.vmulps(tmp, r[x], r[y.id]);
                                     X.vaddps(r[d], tmp, r[z.id]); }
                    break;

                case Op::add_i32: X.vpaddd (r[d], r[x], r[y.id]); break;
                case Op::sub_i32: X.vpsubd (r[d], r[x], r[y.id]); break;
                case Op::mul_i32: X.vpmulld(r[d], r[x], r[y.id]); break;

                case Op::sub_i16x2: X.vpsubw (r[d], r[x], r[y.id]); break;
                case Op::mul_i16x2: X.vpmullw(r[d], r[x], r[y.id]); break;
                case Op::shr_i16x2: X.vpsrlw (r[d], r[x],   y.imm); break;

                case Op::bit_and: X.vandps(r[d], r[x], r[y.id]); break;
                case Op::bit_or : X.vorps (r[d], r[x], r[y.id]); break;
                case Op::bit_xor: X.vxorps(r[d], r[x], r[y.id]); break;

                case Op::shl: X.vpslld(r[d], r[x], y.imm); break;
                case Op::shr: X.vpsrld(r[d], r[x], y.imm); break;
                case Op::sra: X.vpsrad(r[d], r[x], y.imm); break;

                case Op::extract: if (y.imm) {
                                      X.vpsrld(tmp, r[x], y.imm);
                                      X.vandps(r[d], tmp, r[z.id]);
                                  } else {
                                      X.vandps(r[d], r[x], r[z.id]);
                                  }
                                  break;

                case Op::pack: X.vpslld(tmp, r[y.id], z.imm);
                               X.vorps (r[d], tmp, r[x]);
                               break;

                case Op::to_f32: X.vcvtdq2ps (r[d], r[x]); break;
                case Op::to_i32: X.vcvttps2dq(r[d], r[x]); break;

                case Op::bytes: {
                    if (vpshufb_masks.end() == vpshufb_masks.find(y.imm)) {
                        // Translate bytes()'s control nibbles to vpshufb's control bytes.
                        auto nibble_to_vpshufb = [](unsigned n) -> uint8_t {
                            return n == 0 ? 0xff  // Fill with zero.
                                          : n-1;  // Select n'th 1-indexed byte.
                        };
                        uint8_t control[] = {
                            nibble_to_vpshufb( (y.imm >>  0) & 0xf ),
                            nibble_to_vpshufb( (y.imm >>  4) & 0xf ),
                            nibble_to_vpshufb( (y.imm >>  8) & 0xf ),
                            nibble_to_vpshufb( (y.imm >> 12) & 0xf ),
                        };

                        // Now, vpshufb is one of those weird AVX instructions
                        // that does everything in 2 128-bit chunks, so we'll
                        // only really need 4 distinct values to write in our pattern:
                        int p[4];
                        for (int i = 0; i < 4; i++) {
                            p[i] = (int)control[0] <<  0
                                 | (int)control[1] <<  8
                                 | (int)control[2] << 16
                                 | (int)control[3] << 24;

                            // Update each byte that refers to a byte index by 4 to
                            // point into the next 32-bit lane, but leave any 0xff
                            // that fills with zero alone.
                            control[0] += control[0] == 0xff ? 0 : 4;
                            control[1] += control[1] == 0xff ? 0 : 4;
                            control[2] += control[2] == 0xff ? 0 : 4;
                            control[3] += control[3] == 0xff ? 0 : 4;
                        }

                        // Notice, same patterns for top 4 32-bit lanes as bottom.
                        data32.push_back(Data32{Xbyak::Label(), {p[0], p[1], p[2], p[3],
                                                                 p[0], p[1], p[2], p[3]}});
                        vpshufb_masks[y.imm] = data32.size() - 1;
                    }
                    X.vpshufb(r[d], r[x],
                              X.ptr[X.rip + data32[vpshufb_masks[y.imm]].label]);
                } break;
            }
        }

        for (int i = 0; i < nargs; i++) {
            X.add(arg[i], K*(int)strides[i]);
        }
        X.sub(N, K);
        X.jne("loop");

        a.vzeroupper();
        a.ret();

        for (auto data : data4) {
            a.align(4);
            X.L(data.label);
            X.dd(data.bits);
        }
        for (auto data : data32) {
            a.align(32);
            X.L(data.label);
            for (int i = 0; i < 8; i++) {
                X.dd(data.bits[i]);
            }
        }

        // Return mask to apply to N for elements the JIT can handle.
        return ~(K-1);
    }
#endif //defined(SKVM_JIT)

    void Program::eval(int n, void* args[], size_t strides[], int nargs) const {
    #if defined(SKVM_JIT)
        if (!fJIT && can_jit(fRegs, nargs)) {
            fJIT.reset(new Assembler);
            fJITMask = jit(*fJIT, fInstructions, fRegs, fLoop, strides, nargs);
        #if 1
            // We're doing some really stateful things below,
            // so one thread at a time please...
            static SkSpinlock dump_lock;
            SkAutoSpinlock lock(dump_lock);

            uint32_t hash = SkOpts::hash(fJIT->code(), fJIT->size());

            SkString name = SkStringPrintf("skvm-jit-%u", hash);

            // Create a jit-<pid>.dump file that we can `perf inject -j` into a
            // perf.data captured with `perf record -k 1`, letting us see each
            // JIT'd Program as if a function named skvm-jit-<hash>.   E.g.
            //
            //   ninja -C out nanobench
            //   perf record -k 1 out/nanobench -m SkVM_4096_I32\$
            //   perf inject -j -i perf.data -o perf.data.jit
            //   perf report -i perf.data.jit
            //
            // Running `perf inject -j` will also dump an .so for each JIT'd
            // program, named jitted-<pid>-<hash>.so.

            auto timestamp_ns = []() -> uint64_t {
                // It's important to use CLOCK_MONOTONIC here so that perf can
                // correlate our timestamps with those captured by `perf record
                // -k 1`.  That's also what `-k 1` does, by the way, tell perf
                // record to use CLOCK_MONOTONIC.
                struct timespec ts;
                clock_gettime(CLOCK_MONOTONIC, &ts);
                return ts.tv_sec * (uint64_t)1e9 + ts.tv_nsec;
            };

            // We'll open the jit-<pid>.dump file and write a small header once,
            // and just leave it open forever because we're lazy.
            static FILE* jitdump = [&]{
                // Must map as w+ for the mmap() call below to work.
                FILE* f = fopen(SkStringPrintf("jit-%d.dump", getpid()).c_str(), "w+");

                // Calling mmap() on the file adds a "hey they mmap()'d this" record to
                // the perf.data file that will point `perf inject -j` at this log file.
                // Kind of a strange way to tell `perf inject` where the file is...
                void* marker = mmap(nullptr,
                                    sysconf(_SC_PAGESIZE),
                                    PROT_READ|PROT_EXEC,
                                    MAP_PRIVATE,
                                    fileno(f),
                                    /*offset=*/0);
                SkASSERT_RELEASE(marker != MAP_FAILED);
                // Like never calling fclose(f), we'll also just always leave marker mmap()'d.

                struct Header {
                    uint32_t magic, version, header_size, elf_mach, reserved, pid;
                    uint64_t timestamp_us, flags;
                } header = {
                    0x4A695444, 1, sizeof(Header), 62/*x86-64*/, 0, (uint32_t)getpid(),
                    timestamp_ns() / 1000, 0,
                };
                fwrite(&header, sizeof(header), 1, f);

                return f;
            }();

            struct CodeLoad {
                uint32_t event_type, event_size;
                uint64_t timestamp_ns;

                uint32_t pid, tid;
                uint64_t vma/*???*/, code_addr, code_size, id;
            } load = {
                0/*code load*/, (uint32_t)(sizeof(CodeLoad) + name.size() + 1 + fJIT->size()),
                timestamp_ns(),

                (uint32_t)getpid(), (uint32_t)SkGetThreadID(),
                (uint64_t)fJIT->code(), (uint64_t)fJIT->code(), fJIT->size(), hash,
            };

            // Write the header, the JIT'd function name, and the JIT'd code itself.
            fwrite(&load, sizeof(load), 1, jitdump);
            fwrite(name.c_str(), 1, name.size(), jitdump);
            fwrite("\0", 1, 1, jitdump);
            fwrite(fJIT->code(), 1, fJIT->size(), jitdump);
        #endif
        }

        const int jitN = n & fJITMask;
        if (fJIT && jitN > 0) {
            bool ran = true;

            void* fn = fJIT->code();
            switch (nargs) {
                case 0: ((void(*)(int              ))fn)(jitN                  ); break;
                case 1: ((void(*)(int, void*       ))fn)(jitN, args[0]         ); break;
                case 2: ((void(*)(int, void*, void*))fn)(jitN, args[0], args[1]); break;
                default: ran = false; break;
            }
            if (ran) {
                // Step n and arguments forward to where the JIT stopped.
                n -= jitN;

                void**        arg    = args;
                const size_t* stride = strides;
                for (; *arg; arg++, stride++) {
                    *arg = (void*)( (char*)*arg + jitN * *stride );
                }
                SkASSERT(arg == args + nargs);
            }
        }
    #endif
        if (n) {
            SkOpts::eval(fInstructions.data(), (int)fInstructions.size(), fRegs, fLoop,
                         n, args, strides, nargs);
        }
    }
}

// TODO: argument strides (more generally types) should come earlier, the pointers themselves later.
