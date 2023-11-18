package Instructions;

import Debugger.Debugger;
import Expressions.Expression;

public class Print extends Instruction {
    private Expression expression;

    public Print(Expression expression) {
        this.expression = expression;
    }

    @Override
    public void execute(Debugger debugger) throws Exception {
        System.out.println(this.expression.evaluate(debugger));
    }

    @Override
    public void printName() {
        System.out.println("Print out the value of " + this.expression.asString() + ".");
    }
}
