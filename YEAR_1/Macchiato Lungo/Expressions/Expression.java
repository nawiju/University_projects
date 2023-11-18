package Expressions;

import Debugger.Debugger;

public abstract class Expression {

    public abstract int evaluate(Debugger debugger) throws Exception;

    @Override
    public String toString() {
        return asString();
    }

    public abstract String asString();

    protected int getWeight() {
        return Integer.MAX_VALUE;
    }
}
