package Debugger;

import Instructions.*;

import java.util.HashMap;
import java.util.Map;

/**
 * Class holding the valuing of the variables in the scope.
 * Utilizes shallow copy so that one can access the full variable valuing
 * from the top context on the context stack.
 */

public class BlockContext {

    private Variable[] variables;

    private Instruction instruction;

    private HashMap<String, ProcedureDeclaration> procedures = new HashMap<>();

    /**
     * <iteration> utilized when executing the for loop as, according to the specification,
     * the for instruction is an implicit block.
     */
    private int iteration = 0;

    public BlockContext(Instruction instruction, Debugger debugger) {

        if (debugger.getTopContext() == this || debugger.getTopContext() == null) {
            this.variables = new Variable[26];
        } else {
            this.variables = debugger.getTopContext().getVariables().clone();

            for (Map.Entry<String, ProcedureDeclaration> entry : debugger.getTopContext().getProcedures().entrySet()) {
                procedures.put(entry.getKey(), entry.getValue());
            }
        }

        this.instruction = instruction;
    }

    public Variable[] getVariables() {
        return this.variables;
    }

    public ProcedureDeclaration getProcedure(String name) {
        return procedures.get(name);
    }

    public HashMap<String, ProcedureDeclaration> getProcedures() {
        return this.procedures;
    }

    public void replaceVariable(Variable variable) {
        variables[variable.getName() - 'a'] = variable;
    }

    public void addProcedure(ProcedureDeclaration procedureDeclaration) {
        procedures.put(procedureDeclaration.getName(), procedureDeclaration);
    }

    public void removeProcedure(ProcedureDeclaration procedureDeclaration) {
        procedures.remove(procedureDeclaration.getName());
    }
    public void incrementIteration() {
        this.iteration++;
    }

    public int getIteration() {
        return this.iteration;
    }
}