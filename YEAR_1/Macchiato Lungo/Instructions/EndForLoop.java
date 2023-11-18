package Instructions;

import Debugger.Debugger;
import Expressions.Constant;

import java.util.ArrayList;

/**
 * Class holds the instructions of its parent for loop, the number of iterations to be executed
 * and the name of the iterating variable.
 * If the number of iterations (held in the corresponding BlockContext) is smaller than the
 * number of iterations to be executed, the instructions of the for loop are placed onto the
 * instruction stack.
 */

public class EndForLoop extends Instruction {

    private ArrayList<Instruction> instructions;

    private int expressionEval;

    private char name;

    public EndForLoop(ArrayList<Instruction> instructions, int expressionEval, char name) {
        this.instructions = instructions;
        this.expressionEval = expressionEval;
        this.name = name;
    }

    @Override
    public void execute(Debugger debugger) throws Exception {
        try {
            if (debugger.getTopContext().getIteration() < expressionEval) {
                instructions.set(0, new AssignVariableValue(name, Constant.of(debugger.getTopContext().getIteration())));

                for (int j = instructions.size() - 1; j >= 0; j--) {
                    debugger.addInstruction(instructions.get(j));
                }

                debugger.getTopContext().incrementIteration();
            }
        } catch (Exception e) {
            throw e;
        }
    }

    @Override
    public void printName() {
        System.out.println("Reevaluate for-loop condition.");
    }
}
