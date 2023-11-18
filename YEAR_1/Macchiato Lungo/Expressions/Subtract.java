package Expressions;

import Debugger.Debugger;

public class Subtract extends Operation {

    private Subtract(Expression left, Expression right) {
        super(left, right);
    }

    @Override
    public int evaluate(Debugger debugger) throws Exception {
        try {
            return left.evaluate(debugger) - right.evaluate(debugger);
        } catch (Exception e) {
            throw e;
        }
    }

    @Override
    protected int getWeight() {
        return 100;
    }

    @Override
    protected String getSymbol() {
        return "-";
    }

    public static Subtract of(Expression left, Expression right) {
        return new Subtract(left, right);
    }
}
