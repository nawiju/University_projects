package Instructions;
import Debugger.*;
import Expressions.Expression;

import java.util.ArrayList;
import java.util.HashMap;

import static java.lang.Character.isLowerCase;

/**
 * This class is responsible for creating the 'skeleton' of a procedure, ie. holds the instructions to be
 * invoked, the number of variables and whether any errors/exceptions will be triggered when declaring the procedure.
 * On execute(), it will check whether the current scope contains a procedure of the same name, if so, throws an
 * exception, otherwise, it is added to the hash map of procedures of the current scope (ie. block). If there is a
 * procedure of the same name in a higher scope, it will be overshadowed by the new one.
 */

public class ProcedureDeclaration extends Instruction{
    private final String DOUBLE_DECLARATION_PROCEDURE_EXCEPTION = "[Exception] This procedure has already been declared in this scope!";

    private final String DOUBLE_PARAM_EXCEPTION = "[Exception] This parameter has already been declared in this procedure!";

    private final String INVALID_NAME_EXCEPTION = "[Exception] This procedure cannot be named this way!";

    private String name;

    private BlockContext contextDeclared = null;

    private boolean[] parameterPresent = new boolean[26];

    private ArrayList<Character> parameters = new ArrayList<>();

    private ArrayList<Instruction> instructions = new ArrayList<>();

    private boolean doubleParameterDeclaration = false;

    private boolean invalidName = false;

    private ProcedureDeclaration(Builder builder) {
        int i = 0;

        while (i < builder.name.length() && isLowerCase(builder.name.charAt(i))) {
            i++;
        }

        if (i != builder.name.length()) {
            invalidName = true;
        }

        this.name = builder.name;
        this.instructions.addAll(builder.variableDeclarations);
        this.instructions.addAll(builder.instructions);
        this.instructions.add(new EndBlockInstruction());

        for (char parameter : builder.parameters) {
            if (!parameterPresent[parameter - 'a']) {
                parameterPresent[parameter - 'a'] = true;
                parameters.add(parameter);
            } else {
                doubleParameterDeclaration = true;
                parameters.add(parameter);
            }
        }
    }

    public BlockContext getContextDeclared() {
        return this.contextDeclared;
    }

    @Override
    public void execute(Debugger debugger) throws Exception {
        if (doubleParameterDeclaration) {
            throw new Exception(DOUBLE_PARAM_EXCEPTION);
        }

        if (invalidName) {
            throw new Exception(INVALID_NAME_EXCEPTION);
        }

        this.contextDeclared = debugger.getTopContext();

        ProcedureDeclaration procedure = debugger.getTopContext().getProcedure(name);

        if (procedure == null) {
            debugger.getTopContext().addProcedure(this);
        } else if (procedure.getContextDeclared() != debugger.getTopContext()) {
            debugger.getTopContext().removeProcedure(procedure);
            debugger.getTopContext().addProcedure(this);
        } else {
            throw new Exception(DOUBLE_DECLARATION_PROCEDURE_EXCEPTION);
        }
    }

    public String getName() {
        return name;
    }

    public ArrayList<Character> getParameters() {
        return  this.parameters;
    }

    public ArrayList<Instruction> getInstructions() {
        return this.instructions;
    }

    @Override
    public void printName() {
        System.out.println("Declare procedure called '" + name + "' with parameters " + parameters.toString() + ".");
    }

    public static class Builder {

        private String name;

        private final ArrayList<VariableDeclaration> variableDeclarations = new ArrayList<>();

        private final ArrayList<Instruction> instructions = new ArrayList<>();

        private ArrayList<Character> parameters = new ArrayList<>();

        public Builder(String name, char... params) {
            this.name = name;

            for (char parameter : params) {
                this.parameters.add(parameter);
            }
        }

        public Builder condition(IfElseStatement ifElseStatement) {
            this.instructions.add(ifElseStatement);
            return this;
        }

        public Builder declareProcedure(ProcedureDeclaration procedure) {
            instructions.add(procedure);
            return this;
        }

        public Builder declareVariable(char name, Expression expression) {
            variableDeclarations.add(new VariableDeclaration(name, expression));
            return this;
        }

        public Builder forLoop(ForLoop forLoop) {
            instructions.add(forLoop);
            return this;
        }

        public Builder newBlock(Block block) {
            instructions.add(block);
            return this;
        }

        public Builder print(Expression expression) {
            instructions.add(new Print(expression));
            return this;
        }

        public Builder invoke(String name, Expression... parameters) {
            instructions.add(new ProcedureInvoke(name, parameters));
            return this;
        }


        public Builder assign(char name, Expression expression) {
            instructions.add(new AssignVariableValue(name, expression));
            return this;
        }

        public ProcedureDeclaration build() {
            return new ProcedureDeclaration(this);
        }
    }
}
