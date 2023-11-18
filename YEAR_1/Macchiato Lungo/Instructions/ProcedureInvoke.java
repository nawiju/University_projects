package Instructions;

import Debugger.*;
import Expressions.Constant;
import Expressions.Expression;

import java.util.ArrayList;
import java.util.List;

/**
 * This class is responsible for creating the scope for a procedure, evaluating the arguments/parameters in
 * the appropriate order and placing all instructions on the instruction stack to be executed in the appropriate manner.
 * On execute(), this class will create new variables for the procedures parameters (so they overshadow variable of
 * the same name in higehr scopes).
 */

public class ProcedureInvoke extends Instruction {

    private final String NONEXISTENT_PROCEDURE_EXCEPTION = "[Exception] This procedure does not exist in this scope!";

    private final String INVALID_NUMBER_OF_ARGUMENTS = "[Exception] This procedure was called with a different number of arguments than required!";

    private String name;

    private ArrayList<Expression> parameters = new ArrayList<>();

    public ProcedureInvoke(String name, Expression... parameters) {
        this.name = name;
        this.parameters.addAll(List.of(parameters));
    }

    @Override
    public void execute(Debugger debugger) throws Exception {
        ProcedureDeclaration procedure = debugger.getTopContext().getProcedure(name);

        if (procedure == null) {
            throw new Exception(NONEXISTENT_PROCEDURE_EXCEPTION);
        } else {
            debugger.addContext(new BlockContext(this, debugger));

            if (procedure.getParameters().size() != parameters.size()) {
                throw new Exception(INVALID_NUMBER_OF_ARGUMENTS);
            }

            for (int i = 0; i < parameters.size(); i++) {
                Variable newVariable = new Variable(procedure.getParameters().get(i), debugger.getTopContext(), Constant.of(parameters.get(i).evaluate(debugger)));
                debugger.getTopContext().replaceVariable(newVariable);
            }

            for (int i = procedure.getInstructions().size() - 1; i >= 0; i--) {
                debugger.addInstruction(procedure.getInstructions().get(i));
            }
        }
    }

    @Override
    public void printName() {
        System.out.println("Call procedure called '" + name + "' with parameters " + parameters.toString() + ".");
    }
}
