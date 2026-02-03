#include <cstdio>
#include <format>

#include "cpu.h"

CPU::CPU(Bus& busReference) : bus(busReference) {
}

void CPU::step() {
    // 1. Fetch
    Byte opcode = fetchByte();

    // 2. Decode & Execute
    switch (opcode) {
        // NOP
        case 0x00:
            cycles += 4;
            break;
        // LD BC, d16 (Opcode 01)
        case 0x01:
            setBC(fetchWord());
            cycles += 12;
            break;
        case 0x05:
            dec(B); // Helper que ya tenías
            cycles += 4;
            break;

        // LD DE, d16 (Opcode 11)
        case 0x11:
            setDE(fetchWord());
            cycles += 12;
            break;
            // INC DE (Opcode 13) - Incremento de 16 bits
            // Igual que INC HL (23), NO afecta flags.
        case 0x13:
            setDE(getDE() + 1);
            cycles += 8;
            break;

            // INC (HL) (Opcode 34) - Incremento de 8 bits en memoria
        case 0x34:
        {
            // 1. Leer valor actual de memoria
            Byte val = bus.read(getHL());

            // 2. Incrementar (usamos tu helper que maneja Flags Z, N, H)
            inc(val);

            // 3. Escribir de vuelta
            bus.write(getHL(), val);

            cycles += 12;
        }
            break;

            // LD A, E (Opcode 7B) - Copiar registro E en A
        case 0x7B:
            A = E;
            cycles += 4;
            break;

            // CP d8 (Opcode FE) - Comparar A con un valor inmediato
        case 0xFE:
            cp(fetchByte()); // Leemos el siguiente byte y comparamos
            cycles += 8;
            break;

        // LD C, d8 (Opcode 0E)
        case 0x0E:
            C = fetchByte();
            cycles += 8;
            break;
        // INC B (Opcode 04)
        case 0x04:
            inc(B); // ¡Usa tu helper!
            cycles += 4;
            break;
        // LD B, A (Opcode 47) - Copia A en B
        case 0x47:
            B = A;
            cycles += 4;
            break;

        // LD (HL), A (Opcode 77) - Escribe A en la dirección de memoria HL
        case 0x77:
            bus.write(getHL(), A);
            cycles += 8;
            break;

        // --- HIGH RAM & IO (Sonidos, Pantalla) ---
        // LDH (a8), A (Opcode E0) -> Escribe en 0xFF00 + dato inmediato
        case 0xE0:
        {
            Byte offset = fetchByte();
            Word addr = 0xFF00 | offset; // 0xFF00 + offset
            bus.write(addr, A);
            cycles += 12;
        }
            break;

        // LD (C), A (Opcode E2) -> Escribe en 0xFF00 + Registro C
        // Nota: Aunque se escribe (C), la dirección real es FF00 + C
        case 0xE2:
        {
            Word addr = 0xFF00 | C;
            bus.write(addr, A);
            cycles += 8;
        }
            break;
        case 0x06:
            B = fetchByte();
            cycles += 8;
            break;

        case 0x3E:
            A = fetchByte();
            cycles += 8;
            break;
        case 0x80:
            add(B);
            cycles += 4;
            break;
        case 0xC3: {
            Word targetAddress = fetchWord();
            PC = targetAddress;
            cycles += 16; // ~16/12 en documentacion
        }
            break;
            // LD HL, d16 (Cargar valor de 16 bits en HL)
        case 0x21:
            setHL(fetchWord());
            cycles += 12;
            break;

            // LD SP, d16 (Inicializar Stack Pointer)
        case 0x31:
            SP = fetchWord();
            cycles += 12;
            break;

            // LD (HL-), A (Escribir A en (HL) y decrementar HL)
        case 0x32:
            bus.write(getHL(), A);
            setHL(getHL() - 1); // Decrementamos después de escribir
            cycles += 8;
            break;

            // XOR A (Opcode AF)
            // Lógica: A = A ^ A (que es 0). Flags: Z=1.
        case 0xAF:
            A = A ^ A; // Obviamente 0, pero seguimos la lógica formal
            setFlag(F_Z, true); // Resultado es cero
            setFlag(F_N, false);
            setFlag(F_H, false);
            setFlag(F_C, false);
            cycles += 4;
            break;
        case 0x0C: inc(C); cycles += 4; break;
        case 0x20:
        {
            int8_t off = (int8_t)fetchByte();
            if(!getFlag(F_Z)) { PC += off; cycles += 12; }
            else { cycles += 8; }
        }
            break;

            // EI (Enable Interrupts) - 0xFB
            // Por ahora no tenemos interrupciones, pero la BIOS lo pide.
        case 0xFB:
            // TODO: masterInterruptEnable = true;
            cycles += 4;
            break;
            // RLA (Opcode 17) - Rotate Left Accumulator
        case 0x17:
        {
            // Usamos el helper rl(A) pero corregimos el flag Z
            rl(A);
            setFlag(F_Z, false); // ¡Regla especial de RLA!
            cycles += 4;
        }
            break;

        // LD C, A (Opcode 4F) - Copiar A en C
        case 0x4F:
            C = A;
            cycles += 4;
            break;

        // POP BC (Opcode C1) - Sacar del Stack a BC
        case 0xC1:
            setBC(popStack());
            cycles += 12;
            break;

        // PUSH BC (Opcode C5) - Guardar BC en el Stack
        case 0xC5:
            pushStack(getBC());
            cycles += 16;
            break;
        case 0xCB:
        {
            Byte cbOp = fetchByte();
            cycles += 4;
            switch(cbOp) {
                // RL C (Opcode 11 dentro de CB) -> Rotate Left C
                case 0x11:
                    rl(C);
                    cycles += 8;
                    break;

                    // BIT 7, H (0x7C) que ya tenías...
                case 0x7C: bit(7, H); cycles += 8; break;

                default:
                    std::cout << std::format("Unimplemented CB: {:02X}\n", cbOp);
            }
        }
            break;
        // LD A, (DE) - Opcode 1A
        // Lee el byte apuntado por el registro DE y lo guarda en A
        case 0x1A:
            A = bus.read(getDE());
            cycles += 8;
            break;

        // SUB L - Opcode 95
        // Resta el valor de L al registro A
        case 0x95:
            sub(L); // Usamos el helper
            cycles += 4;
            break;

        // CALL a16 - Opcode CD
        // Llama a una subrutina
        case 0xCD:
        {
            // 1. Leemos la dirección de destino (donde queremos ir)
            Word targetAddr = fetchWord();

            // 2. Guardamos el PC actual en el Stack para saber volver.
            // IMPORTANTE: El PC ya avanzó (gracias a fetchWord), así que
            // guardamos la dirección de la SIGUIENTE instrucción.
            pushStack(PC);

            // 3. Saltamos
            PC = targetAddr;

            cycles += 24; // Call es costoso
        }
            break;
            // RET (Opcode C9) - Return from Subroutine
        case 0xC9:
            PC = popStack(); // Recuperamos donde estábamos
            cycles += 16;
            break;
            // LD (HL+), A (Opcode 22) - También llamada LDI (Load and Increment)
        case 0x22:
            bus.write(getHL(), A); // Escribir
            setHL(getHL() + 1);    // Incrementar HL
            cycles += 8;
            break;
            // INC HL (Opcode 23) - 16-bit Increment
        case 0x23:
            setHL(getHL() + 1); // Simple suma, SIN tocar flags
            cycles += 8;
            break;
            // DEC C (Opcode 0D)
        case 0x0D:
            dec(C); // Helper con flags correctos
            cycles += 4;
            break;
            // JR r8 (Opcode 18) - Salto Relativo Incondicional
        case 0x18:
        {
            int8_t offset = (int8_t)fetchByte();
            PC += offset;
            cycles += 12;
        }
            break;

            // JR Z, r8 (Opcode 28) - Salto si Z es 1
        case 0x28:
        {
            int8_t offset = (int8_t)fetchByte();
            if (getFlag(F_Z)) {
                PC += offset;
                cycles += 12;
            } else {
                cycles += 8;
            }
        }
            break;
            // LDH A, (a8) (Opcode F0) - Leer de 0xFF00 + n
        case 0xF0:
        {
            Byte offset = fetchByte();
            Word addr = 0xFF00 | offset;
            A = bus.read(addr);
            cycles += 12;
        }
            break;
            // LD (a16), A (Opcode EA)
        case 0xEA:
        {
            Word addr = fetchWord();
            bus.write(addr, A);
            cycles += 16;
        }
            break;
            // SBC A, C (Opcode 99)
        case 0x99:
            sbc(C);
            cycles += 4;
            break;
            // STOP (Opcode 10)
        case 0x10:
            fetchByte(); // Consumimos el byte extra (00) que sigue al STOP
            // Aquí deberíamos pausar la CPU, pero por ahora...
            std::cout << "STOP Instruction executed.\n";
            cycles += 4;
            break;
            // DEC A (Opcode 3D)
        case 0x3D:
            dec(A);
            cycles += 4;
            break;
            // LD H, A (Opcode 67) - Copia A en H
        case 0x67:
            H = A;
            cycles += 4;
            break;

            // LD D, A (Opcode 57) - Copia A en D
        case 0x57:
            D = A;
            cycles += 4;
            break;
            // LD E, d8 (Opcode 1E)
        case 0x1E:
            E = fetchByte();
            cycles += 8;
            break;
            // LD (BC), A (Opcode 02)
        case 0x02:
            bus.write(getBC(), A);
            cycles += 8;
            break;
            // LD L, d8 (Opcode 2E)
        case 0x2E:
            L = fetchByte();
            cycles += 8;
            break;
            // RRCA (Opcode 0F) - Rotate Right Circular Accumulator
        case 0x0F:
        {
            // 1. Guardamos el bit 0 (que se va a caer)
            Byte bit0 = A & 0x01;

            // 2. Rotamos: A >> 1 y metemos el bit0 en la posición 7
            A = (A >> 1) | (bit0 << 7);

            // 3. Flags (Truco: Z siempre false en RRCA)
            setFlag(F_Z, false);
            setFlag(F_N, false);
            setFlag(F_H, false);
            setFlag(F_C, bit0); // El bit que salió va al Carry

            cycles += 4;
        }
            break;
        default:
            printf("Unhandled opcode: %02x\n", opcode);
            break;
    }
}

void CPU::reset() {
    PC = 0x0100; // Punto de entrada estándar de la GB
    // TODO: valores por defecto de todos los registros.
}

// FETCH
Byte CPU::fetchByte() {
    Byte data = bus.read(PC);
    PC++;
    return data;
}

Word CPU::fetchWord() {
    Byte data1 = bus.read(PC);
    PC++;
    Byte data2 = bus.read(PC);
    PC++;
    Word data = (data2 << 8) | data1;
    return data;
}

void CPU::add(Byte value) {
    Word result = A + value;

    // Flags
    setFlag(F_Z, (result & 0xFF) == 0);
    setFlag(F_N, false);
    setFlag(F_H, ((A & 0x0F) + (value & 0x0F)) > 0x0F);
    setFlag(F_C, result > 0xFF);

    A = static_cast<Byte>(result & 0xFF);
}

void CPU::inc(Byte& reg) {
    // Antes de sumar, miramos el Half Carry
    // Si los 4 bits bajos son F (15), al sumar 1 habrá desborde al bit 4
    setFlag(F_H, (reg & 0x0F) == 0x0F);

    // Sumamos
    reg++;

    // Flags restantes
    setFlag(F_Z, reg == 0);
    setFlag(F_N, false);
    // INC no toca el Flag C
}

void CPU::dec(Byte& reg) {
    // Half Carry en resta (Borrow): Si los 4 bits bajos son 0, al restar pedirá prestado
    setFlag(F_H, (reg & 0x0F) == 0x00);

    reg--;

    setFlag(F_Z, reg == 0);
    setFlag(F_N, true); // Es una resta
    // DEC tampoco toca C
}

void CPU::bit(int bitIndex, Byte regVal) {
    bool isZero = !((regVal >> bitIndex) & 1);
    setFlag(F_Z, isZero);

    // Flags fijos según manual
    setFlag(F_N, false);
    setFlag(F_H, true); // BIT siempre enciende H
    // C no se toca
}

void CPU::sub(Byte value) {
    // 1. Validar Carry (Borrow) antes de restar
    // Si A es menor que value, necesitaremos un "préstamo", así que Carry = true
    setFlag(F_C, A < value);

    // 2. Validar Half-Carry (Borrow del nibble bajo)
    // ((A & 0xF) - (value & 0xF)) < 0
    setFlag(F_H, (A & 0x0F) < (value & 0x0F));

    // 3. Realizar la resta
    A -= value;

    setFlag(F_Z, A == 0);
    setFlag(F_N, true);
}

void CPU::pushStack(Word value) {
    // El Stack guarda 2 bytes (16 bits).
    // Primero guardamos el byte ALTO, luego el BAJO.
    // Decrementamos SP antes de escribir cada byte.

    // 1. High Byte
    SP--;
    bus.write(SP, (value >> 8) & 0xFF);

    // 2. Low Byte
    SP--;
    bus.write(SP, value & 0xFF);
}

Word CPU::popStack() {
    // 1. Leemos el Byte BAJO primero (porque el Stack es LIFO - Last In First Out)
    Byte lo = bus.read(SP);
    SP++;

    // 2. Leemos el Byte ALTO
    Byte hi = bus.read(SP);
    SP++;

    // 3. Combinamos
    return (hi << 8) | lo;
}

void CPU::rl(Byte& reg) {
    // 1. Guardamos el bit 7 actual (será el nuevo Carry)
    bool isCarry = (reg & 0x80) != 0;

    // 2. Recuperamos el Carry viejo (entrará por la derecha)
    bool oldCarry = getFlag(F_C);

    // 3. Hacemos el shift y metemos el carry viejo en el bit 0
    reg = (reg << 1) | (oldCarry ? 1 : 0);

    // 4. Actualizamos Flags
    setFlag(F_Z, reg == 0); // Z se enciende si el resultado es 0
    setFlag(F_N, false);
    setFlag(F_H, false);
    setFlag(F_C, isCarry); // El bit 7 que salió es el nuevo Carry
}

void CPU::cp(Byte value) {
    // La lógica es IDÉNTICA a SUB, pero sin modificar A.

    // 1. Validar Carry (Si A < value, necesitamos préstamo)
    setFlag(F_C, A < value);

    // 2. Validar Half-Carry (Borrow del nibble bajo)
    setFlag(F_H, (A & 0x0F) < (value & 0x0F));

    // 3. Flag Z (Si son iguales, la resta daría 0)
    setFlag(F_Z, A == value);

    // 4. Flag N (Siempre es una resta)
    setFlag(F_N, true);

    // ¡OJO! No hacemos "A -= value". A se queda intacto.
}

void CPU::sbc(Byte value) {
    // Recuperamos el Carry actual (0 o 1)
    Byte carry = getFlag(F_C) ? 1 : 0;

    // Resultado matemático amplio para chequear overflow
    // OJO: Es (A - val - carry)
    int result = A - value - carry;

    // H Flag: Se enciende si hay préstamo en el bit 4
    // La fórmula compleja de Game Boy para esto es:
    setFlag(F_H, ((A & 0x0F) - (value & 0x0F) - carry) < 0);

    // C Flag: Se enciende si hubo préstamo global (resultado negativo)
    setFlag(F_C, result < 0);

    // Z Flag
    setFlag(F_Z, (result & 0xFF) == 0);

    // N Flag: Siempre 1 en resta
    setFlag(F_N, true);

    A = static_cast<Byte>(result & 0xFF);
}