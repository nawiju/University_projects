package Instructions;

import Debugger.*;
import Expressions.*;

public class AssignVariableValue extends Instruction {

    private final String UNKNOWN_VARIABLE_EXCEPTION = "[Exception] This variable has not been declared!";

    private final String INVALID_NAME_EXCEPTION = "[Exception] This variable name is not allowed in Macchiato!";

    private char name;

    private Expression expression;

    public AssignVariableValue(char name, Expression expression) {
        this.name = name;
        this.expression = expression;
    }

    @Override
    public void printName() {
        System.out.println("Assign the variable '" + name + "' the value of " + expression.asString() + ".");
    }

    @Override
    public void execute(Debugger debugger) throws Exception {

        if (!(this.name <= 'z' && this.name >= 'a')) {
            throw new Exception(INVALID_NAME_EXCEPTION);
        } else {
            if (debugger.getTopContext().getVariables()[this.name - 'a'] == null) {
                throw new Exception(UNKNOWN_VARIABLE_EXCEPTION);
            } else {
                Constant value = Constant.of(expression.evaluate(debugger));
                debugger.getTopContext().getVariables()[this.name - 'a'].setExpression(value);
            }
        }
    }
}
