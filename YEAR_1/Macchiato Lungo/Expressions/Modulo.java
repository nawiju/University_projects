package Expressions;

import Debugger.Debugger;

public class Modulo extends Operation {

    private final String EXCEPTION_DIVIDE_BY_ZERO = "[Exception] You cannot modulo by zero!";

    private Modulo(Expression left, Expression right) {
        super(left, right);
    }

    @Override
    public int evaluate(Debugger debugger) throws Exception {
        if (right.evaluate(debugger) == 0) {
            throw new Exception(EXCEPTION_DIVIDE_BY_ZERO);
        }

        return left.evaluate(debugger) % right.evaluate(debugger);
    }

    @Override
    protected int getWeight() {
        return 1000;
    }

    @Override
    protected String getSymbol() {
        return "%";
    }

    public static Modulo of(Expression left, Expression right) {
        return new Modulo(left, right);
    }
}
