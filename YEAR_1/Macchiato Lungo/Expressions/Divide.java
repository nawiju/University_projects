package Expressions;

import Debugger.Debugger;

public class Divide extends Operation{

    private final String EXCEPTION_DIVIDE_BY_ZERO = "[Exception] You cannot divide by zero!";

    private Divide(Expression left, Expression right) {
        super(left, right);
    }

    @Override
    public int evaluate(Debugger debugger) throws Exception {
        try {
            if (right.evaluate(debugger) == 0) {
                throw new Exception(EXCEPTION_DIVIDE_BY_ZERO);
            }

            return (int) Math.floor((double) left.evaluate(debugger) / right.evaluate(debugger));
        } catch (Exception e) {
            throw e;
        }
    }

    @Override
    protected int getWeight() {
        return 1000;
    }

    @Override
    protected String getSymbol() {
        return "/";
    }

    public static Divide of(Expression left, Expression right) {
        return new Divide(left, right);
    }
}
