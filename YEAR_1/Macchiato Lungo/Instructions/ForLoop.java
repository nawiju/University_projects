package Instructions;

import Debugger.*;
import Expressions.Constant;
import Expressions.Expression;

import java.util.ArrayList;

public class ForLoop extends Instruction {

    private ArrayList<Instruction> instructions = new ArrayList<Instruction>();;

    private char iterator;

    private int expressionEval;

    private Expression expression;

    @Override
    public void printName() {
        System.out.println("Enter for loop for " + iterator + " from 0 to " + expression.toString() + ".");
    }

    public ForLoop(Builder builder) {
        this.iterator = builder.iterator;
        this.expression = builder.expression;
        this.instructions.add(new VariableDeclaration(builder.iterator, Constant.of(0)));
        this.instructions.addAll(builder.instructions);
    }

    @Override
    public void execute(Debugger debugger) throws Exception {
        expressionEval = expression.evaluate(debugger);

        debugger.addContext(new BlockContext(this, debugger));
        debugger.addInstruction(new EndBlockInstruction());

        instructions.set(0, new VariableDeclaration(iterator, Constant.of(0)));
        this.instructions.add(new EndForLoop(instructions, expressionEval, iterator));

        if (debugger.getTopContext().getIteration() < expressionEval) {
            for (int j = instructions.size() - 1; j >= 0; j--) {
                debugger.addInstruction(instructions.get(j));
            }

            debugger.getTopContext().incrementIteration();
        }
    }

    public static class Builder {

        private char iterator;

        private Expression expression;

        private final ArrayList<Instruction> instructions = new ArrayList<>();

        public Builder forLoop(ForLoop forLoop) {
            instructions.add(forLoop);
            return this;
        }

        public Builder condition(IfElseStatement ifElseStatement) {
            this.instructions.add(ifElseStatement);
            return this;
        }

        public Builder(char iterator, Expression expression) {
            this.iterator = iterator;
            this.expression = expression;
        }

        public Builder invoke(String name, Expression... parameters) {
            instructions.add(new ProcedureInvoke(name, parameters));
            return this;
        }

        public Builder newBlock(Block block) {
            instructions.add(block);
            return this;
        }

        public Builder print(Expression expression) {
            instructions.add(new Print(expression));
            return this;
        }

        public Builder assign(char name, Expression expression) {
            instructions.add(new AssignVariableValue(name, expression));
            return this;
        }

        public ForLoop build(){
            return new ForLoop(this);
        }
    }
}
