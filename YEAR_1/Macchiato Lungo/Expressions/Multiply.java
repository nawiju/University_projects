package Expressions;

import Debugger.*;

public class Multiply extends Operation {

    protected Multiply(Expression left, Expression right) {
        super(left, right);
    }

    @Override
    public int evaluate(Debugger debugger) throws Exception {
        try {
            return left.evaluate(debugger) * right.evaluate(debugger);
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
        return "*";
    }

    public static Multiply of(Expression left, Expression right) {
        return new Multiply(left, right);
    }
}