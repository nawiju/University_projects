package Debugger;

import Instructions.*;

import java.util.Arrays;

public class InstructionStack {
    private final String TOO_FEW_INSTRUCTIONS = "[Exception] There are fewer instructions than the number inputted.";

    private Instruction[] instructions = new Instruction[10];

    private int topIndex= 0;

    public void push(Instruction instruction) {
        if (topIndex == instructions.length) {
            instructions = Arrays.copyOf(instructions, instructions.length * 2);
        }
        instructions[topIndex++] = instruction;
    }

    public Instruction peek() throws Exception {
        if (topIndex > 0) {
            return instructions[topIndex - 1];
        } else {
            throw new Exception(TOO_FEW_INSTRUCTIONS);
        }

    }

    public boolean empty() {
        return topIndex == 0;
    }

    public Instruction pop() throws Exception {
        if (topIndex > 0) {
            return instructions[--topIndex];
        } else {
            throw new Exception(TOO_FEW_INSTRUCTIONS);
        }
    }
}

