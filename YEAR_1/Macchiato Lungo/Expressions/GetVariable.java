package Expressions;

import Debugger.*;

/**
 * Method evaluate returns the value of the variable named 'name'.
 * Throws exception if this variable has not been declared in the scope.
 */
public class GetVariable extends Expression {

    private final String UNKNOWN_VARIABLE_EXCEPTION = "[Exception] This variable has not been declared!";

    private char name;

    private int value;

    private GetVariable(char name) {
        this.name = name;
    }

    @Override
    public int evaluate(Debugger debugger) throws Exception {
        try {
            if (debugger.getTopContext().getVariables()[name - 'a'] == null) {
                System.out.println(name + " is trying to be declared.");
                throw new Exception(UNKNOWN_VARIABLE_EXCEPTION);
            } else {
                this.value = debugger.getTopContext().getVariables()[name - 'a'].getExpressionValue(debugger);
            }

            return value;
        } catch (Exception e) {
            throw e;
        }
    }

    @Override
    public String asString() {
        return String.valueOf(name);
    }

    public static GetVariable named(char name) {
        return new GetVariable(name);
    }
}
