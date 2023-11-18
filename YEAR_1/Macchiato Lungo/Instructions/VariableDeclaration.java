package Instructions;

import Debugger.*;
import Expressions.*;

public class VariableDeclaration extends Instruction {
    private final String NONEXISTENT_VARIABLE_EXCEPTION = "[Exception] This variable does not exist in the Macchiato language!";

    private final String DOUBLE_DECLARATION_EXCEPTION = "[Exception] This variable has already been declared in this block!";

    private char name;
    private Expression expression;

    public VariableDeclaration(char name, Expression expression) {
        this.name = name;
        this.expression = expression;
    }

    @Override
    public void execute(Debugger debugger) throws Exception {
        if (!(name <= 'z' && name >= 'a')) {
            throw new Exception(NONEXISTENT_VARIABLE_EXCEPTION);
        } else {
            if ((debugger.getTopContext().getVariables()[name - 'a'] == null) || (debugger.getTopContext().getVariables()[name - 'a'].getContextDeclared() != debugger.getTopContext())) {
                Constant value = Constant.of(expression.evaluate(debugger));
                debugger.getTopContext().getVariables()[name - 'a'] = new Variable(name, debugger.getTopContext(), value);
            } else if (debugger.getTopContext().getVariables()[name - 'a'].getContextDeclared() == debugger.getTopContext()) {
                throw new Exception(DOUBLE_DECLARATION_EXCEPTION);
            }
        }
    }

    @Override
    public void printName() {
        System.out.println("Declare variable '" + name + "' with the value of " + expression.asString() + ".");
    }
}