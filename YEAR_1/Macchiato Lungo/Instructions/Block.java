package Instructions;

import Debugger.*;
import Expressions.Expression;

import java.util.ArrayList;

public class Block extends Instruction {

    protected ArrayList<Instruction> instructions = new ArrayList<>();

    public Block(Builder builder) {
        this.instructions.addAll(builder.variableDeclarations);
        this.instructions.addAll(builder.procedureDeclarations);
        this.instructions.addAll(builder.instructions);
        this.instructions.add(new EndBlockInstruction());
    }

    @Override
    public void printName() {
        System.out.println("Enter block.");
    }

    @Override
    public void execute(Debugger debugger) throws Exception {
        debugger.addContext(new BlockContext(this, debugger));

        for (int i = instructions.size() - 1; i >= 0; i--) {
            debugger.addInstruction(instructions.get(i));
        }
    }

    public static class Builder {
        private final ArrayList<VariableDeclaration> variableDeclarations = new ArrayList<>();

        private final ArrayList<ProcedureDeclaration> procedureDeclarations = new ArrayList<>();

        private final ArrayList<Instruction> instructions = new ArrayList<>();

        public Builder declareVariable(char name, Expression expression) {
            variableDeclarations.add(new VariableDeclaration(name, expression));
            return this;
        }

        public Builder declareProcedure(ProcedureDeclaration procedure) {
            instructions.add(procedure);
            return this;
        }

        public Builder forLoop(ForLoop forLoop) {
            instructions.add(forLoop);
            return this;
        }

        public Builder condition(IfElseStatement ifElseStatement) {
            this.instructions.add(ifElseStatement);
            return this;
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

        public Block build(){
            return new Block(this);
        }
    }
}
