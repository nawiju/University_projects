package Expressions;

import Debugger.Debugger;

/**
 * Integer literal as specified in the program specification.
 */

public class Constant extends Expression {
    private final int value;

    private Constant(int value) {
        this.value = value;
    }

    @Override
    public int evaluate(Debugger debugger) {
        return value;
    }

    @Override
    public String asString() {
        if (value < 0) {
            return "(" + value + ")";
        }
        return "" + value;
    }

    public static Constant of(int value) {
        return new Constant(value);
    }
}
