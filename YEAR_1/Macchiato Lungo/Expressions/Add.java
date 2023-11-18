package Expressions;

import Debugger.Debugger;

public class Add extends Operation {
    private Add(Expression left, Expression right) {
        super(left, right);
    }

    @Override
    public int evaluate(Debugger debugger) throws Exception {
        try {
            return left.evaluate(debugger) + right.evaluate(debugger);
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
        return "+";
    }

    public static Add of(Expression left, Expression right) {
        return new Add(left, right);
    };
}
