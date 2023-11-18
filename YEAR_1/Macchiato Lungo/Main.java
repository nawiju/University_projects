import Debugger.Debugger;
import Expressions.*;
import Instructions.*;

/**
 * Example use of the debugger and builder.
 */

public class Main {

    public static void main(String[] args) throws Exception {
        Block exampleProgram1 = new Block.Builder()
                .declareVariable('x', Constant.of(57))
                .declareVariable('y', Constant.of(15))
                .declareProcedure(new ProcedureDeclaration.Builder("out", 'a')
                        .print(GetVariable.named('a'))
                        .build())
                .assign('x', Subtract.of(GetVariable.named('x'), GetVariable.named('y')))
                .invoke("out", GetVariable.named('x'))
                .invoke("out", Constant.of(125))
                .build();

        new Debugger(exampleProgram1).runProgram();

        Block exampleProgram2 = new Block.Builder()
                .declareVariable('x', Constant.of(101))
                .declareVariable('y', Constant.of(1))
                .declareProcedure(new ProcedureDeclaration.Builder("out", 'a')
                        .print(Add.of(GetVariable.named('a'), GetVariable.named('x')))
                        .build())
                .assign('x', Subtract.of(GetVariable.named('x'), GetVariable.named('y')))
                .invoke("out", GetVariable.named('x'))
                .invoke("out", Constant.of(100))
                .newBlock(new Block.Builder()
                        .declareVariable('x', Constant.of(10))
                        .invoke("out", Constant.of(100))
                        .build())
                .build();

        new Debugger(exampleProgram2).runProgram();
    }
}
