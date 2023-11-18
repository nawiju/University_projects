package Instructions;

import Debugger.*;
import Expressions.Expression;

import java.util.ArrayList;
import java.util.List;

public class IfElseStatement extends Instruction {
    private ArrayList<Instruction> instructionsTrue;
    private ArrayList<Instruction> instructionsFalse;
    private boolean expressionEvaluated;
    private Expression expression1;
    private Expression expression2;
    private Condition condition;


    private IfElseStatement(Expression expression1, Expression expression2, Condition condition, Instruction[] instructionsListTrue, Instruction[] instructionsListFalse) throws Exception {
        this.instructionsTrue = new ArrayList<Instruction>();
        this.instructionsFalse = new ArrayList<Instruction>();

        this.instructionsTrue.addAll(List.of(instructionsListTrue));
        this.instructionsFalse.addAll(List.of(instructionsListFalse));

        this.expression1 = expression1;
        this.expression2 = expression2;
        this.condition = condition;
    }

    private IfElseStatement(Builder builder) {
        this.expression1 = builder.expression1;
        this.expression2 = builder.expression2;;
        this.condition = builder.condition;
        this.instructionsTrue = builder.instructionsTrue;
        this.instructionsFalse = builder.instructionsFalse;
    }

    @Override
    public void printName() {
        System.out.println("Enter an If-Else instruction with the condition: " + expression1.toString() + " " + condition.toString() + " " + expression2.toString() + ".");
    }

    private boolean evaluateExpression (Expression expression1, Expression expression2, Condition condition, Debugger debugger) throws Exception {
        switch (condition) {
            case EQUALS -> {
                return expression1.evaluate(debugger) == expression2.evaluate(debugger);
            }
            case NOT_EQUAL -> {
                return expression1.evaluate(debugger) != expression2.evaluate(debugger);
            }
            case LESS_THAN -> {
                return expression1.evaluate(debugger) < expression2.evaluate(debugger);
            }
            case GREATER_THAN -> {
                return expression1.evaluate(debugger) > expression2.evaluate(debugger);
            }
            case GREATER_EQUAL -> {
                return expression1.evaluate(debugger) >= expression2.evaluate(debugger);
            }
            case SMALLER_EQUAL -> {
                return expression1.evaluate(debugger) <= expression2.evaluate(debugger);
            }
        }
        return false;
    }

    @Override
    public void execute(Debugger debugger) throws Exception {
        try {
            expressionEvaluated = evaluateExpression(expression1, expression2, condition, debugger);

            if (expressionEvaluated) {
                for (int i = instructionsTrue.size() - 1; i >= 0; i--) {
                    debugger.addInstruction(instructionsTrue.get(i));
                }
            } else {
                for (int i = instructionsFalse.size() - 1; i >= 0; i--) {
                    debugger.addInstruction(instructionsFalse.get(i));
                }
            }

        } catch (Exception e) {
            throw e;
        }
    }

    public static class Builder {
        private ArrayList<Instruction> instructionsTrue = new ArrayList<>();

        private ArrayList<Instruction> instructionsFalse = new ArrayList<>();

        private Expression expression1;

        private Expression expression2;

        private Condition condition;

        public Builder(Expression expression1, Condition condition, Expression expression2) {
            this.expression1 = expression1;
            this.expression2 = expression2;
            this.condition = condition;
        }

        public Builder forLoopIfTrue(ForLoop forLoop) {
            instructionsTrue.add(forLoop);
            return this;
        }

        public Builder conditionIfTrue(IfElseStatement ifElseStatement) {
            this.instructionsTrue.add(ifElseStatement);
            return this;
        }

        public Builder invokeIfTrue(String name, Expression... parameters) {
            instructionsTrue.add(new ProcedureInvoke(name, parameters));
            return this;
        }

        public Builder newBlockIfTrue(Block block) {
            instructionsTrue.add(block);
            return this;
        }

        public Builder printIfTrue(Expression expression) {
            instructionsTrue.add(new Print(expression));
            return this;
        }

        public Builder assignIfTrue(char name, Expression expression) {
            instructionsTrue.add(new AssignVariableValue(name, expression));
            return this;
        }

        public Builder forLoopIfFalse(ForLoop forLoop) {
            instructionsFalse.add(forLoop);
            return this;
        }

        public Builder conditionIfFalse(IfElseStatement ifElseStatement) {
            this.instructionsFalse.add(ifElseStatement);
            return this;
        }

        public Builder invokeIfFalse(String name, Expression... parameters) {
            instructionsFalse.add(new ProcedureInvoke(name, parameters));
            return this;
        }

        public Builder newBlockIfFalse(Block block) {
            instructionsFalse.add(block);
            return this;
        }

        public Builder printIfFalse(Expression expression) {
            instructionsFalse.add(new Print(expression));
            return this;
        }

        public Builder assignIfFalse(char name, Expression expression) {
            instructionsFalse.add(new AssignVariableValue(name, expression));
            return this;
        }

        public IfElseStatement build(){
            return new IfElseStatement(this);
        }
    }
}
