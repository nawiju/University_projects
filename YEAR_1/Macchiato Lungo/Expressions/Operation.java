package Expressions;

public abstract class Operation extends Expression {

    protected final Expression left;
    protected final Expression right;

    protected Operation(Expression left, Expression right) {
        this.left = left;
        this.right = right;
    }

    @Override
    public String asString() {
        String str = "";
        if (left.getWeight() < getWeight()) {
            str += "(" + left + ")";
        } else {
            str += left.toString();
        }

        str += getSymbol();

        if (right.getWeight() < getWeight()) {
            str += "(" + right + ")";
        } else {
            str += right.toString();
        }

        return str;
    }

    protected abstract String getSymbol();
}
