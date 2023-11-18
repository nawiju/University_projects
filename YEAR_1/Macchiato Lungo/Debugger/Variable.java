package Debugger;

import Expressions.Expression;

public class Variable {
    private char name;

    private BlockContext contextDeclared = null;

    private Expression expression;

    public Variable(char name, BlockContext context, Expression expression) {
        this.name = name;
        this.contextDeclared = context;
        this.expression = expression;
    }

    public void setExpression(Expression expression) {
        this.expression = expression;
    }

    public int getExpressionValue(Debugger debugger) throws Exception {
        return this.expression.evaluate(debugger);
    }

    public BlockContext getContextDeclared() {
        return this.contextDeclared;
    }

    public char getName() {
        return this.name;
    }
}
