package Debugger;

import Instructions.*;

import java.util.Map;
import java.util.Scanner;
import java.io.FileWriter;

/**
 * Class responsible for the running and debugging of the Macchiato program provided via input.
 * Instance contains the program to be executed and debugged as well as debug methods.
 */

public class Debugger {

    private final String INVALID_COMMAND_EXCEPTION = "[Exception] The inputted command does not exist.";

    private ContextStack contextStack;

    private InstructionStack instructionStack;

    private boolean breakFlag = false;

    public Debugger(Block block) {
        this.contextStack = new ContextStack();
        this.instructionStack = new InstructionStack();
        instructionStack.push(block);
    }

    public BlockContext getTopContext() {
        return contextStack.peek();
    }

    public void addInstruction(Instruction instruction) {
        instructionStack.push(instruction);
    }

    public void addContext(BlockContext context) {
        contextStack.push(context);
    }

    public void removeTopContext() throws Exception {
        contextStack.pop();
    }

    /**
     * Performs one "step" in the program. Throws exception if there are no instructions left to be executed.
     * @throws Exception
     */
    private void step() throws Exception {
        Instruction currentInstruction = this.instructionStack.pop();

        try {
            currentInstruction.execute(this);
        } catch (Exception e) {
            instructionStack.push(currentInstruction);
            displayVariables(0);
            throw e;
        }
    }

    /**
     * Performs <steps> number of steps.
     * @param steps
     * @throws Exception
     */
    private void manySteps(int steps) throws Exception {
        for (int i = 0; i < steps; i++) {
            try {
                step();
            } catch (Exception e) {
                System.out.println(e.getMessage());
                breakFlag = true;
                break;
            }
        }

        if (!instructionStack.empty()) {
            instructionStack.peek().printName();
        } else {
            continueRun();
        }
    }

    // Used for tests.
    public void steps(int n) throws Exception {
        manySteps(n);
    }

    private void continueRun() throws Exception {
        while (!instructionStack.empty()) {
            try {
                step();
            } catch (Exception e) {
                System.out.println(e.getMessage());
                instructionStack.peek().printName();
                breakFlag = true;
                break;
            }
        }

        System.out.println("The program has finished running.");
    }

    private void displayVariables(int levels) throws Exception {
        for (Variable v : contextStack.fetch(levels).getVariables()) {
            if (v != null) {
                System.out.println(v.getName() + ": " + v.getExpressionValue(this));
            }
        }
    }

    private void fileDump(String filePath) throws Exception {
        FileWriter outputFile = new FileWriter(filePath);

        outputFile.write("Variables in scope:\n");

        for (Variable v : contextStack.fetch(0).getVariables()) {
            if (v != null) {
                outputFile.write(v.getName() + " " + v.getExpressionValue(this) + " " + "\n");
            }
        }

        outputFile.write("Procedures in scope:\n");

        for (Map.Entry<String, ProcedureDeclaration> procedure : contextStack.fetch(0).getProcedures().entrySet()) {
            outputFile.write(procedure.getKey() + "\n");
        }

        outputFile.close();
    }

    // Used for tests.
    public void dump(String filePath) throws Exception {
        fileDump(filePath);
    }

    private void runDebugger() throws Exception {
        Scanner StdIn = new Scanner(System.in);

        while (!instructionStack.empty() && !breakFlag) {
            String command = StdIn.nextLine();

            switch (command.charAt(0)) {
                case 'c' -> continueRun();
                case 'e' -> breakFlag = true;
                case 's', 'd', 'm' -> {
                    String[] commands = command.split(" ");

                    if (commands.length != 2) {
                        throw new Exception(INVALID_COMMAND_EXCEPTION);
                    }

                    try {
                        switch (command.charAt(0)) {
                            case 's' -> {
                                int value = Integer.parseInt(commands[1]);
                                manySteps(value);
                            }
                            case 'd' -> {
                                int value = Integer.parseInt(commands[1]);
                                displayVariables(value);
                            }
                            case 'm' -> {
                                String filePath = commands[1];
                                fileDump(filePath);
                            }
                        }
                    } catch (NumberFormatException nfe) {
                        System.out.println("Invalid integer.");
                    }
                }
                default -> throw new Exception(INVALID_COMMAND_EXCEPTION);
            }
        }
    }

    // Simulates someone typing "n" when calling the Debugger. Used in tests.
    public void runDebuggerTest() {
        try {
            continueRun();
            breakFlag = true;
        } catch (Exception e) {
            System.out.println(e.getMessage());
            breakFlag = true;
        }
    }

    public void runProgram() {
        Scanner StdIn = new Scanner(System.in);

        System.out.print("Would you like to run your program with the debugger? [y/n] ");

        while (!breakFlag) {
            String runWithDebugger = StdIn.nextLine();
            try {
                if ("y".equalsIgnoreCase(runWithDebugger)) {
                    runDebugger();
                    breakFlag = true;
                } else if ("n".equalsIgnoreCase(runWithDebugger)) {
                    continueRun();
                    breakFlag = true;
                } else {
                    throw new Exception(INVALID_COMMAND_EXCEPTION);
                }
            } catch (Exception e) {
                System.out.println(e.getMessage());
                breakFlag = true;
            }
        }
    }
}