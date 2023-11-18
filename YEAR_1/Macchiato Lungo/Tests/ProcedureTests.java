package Tests;
import Debugger.Debugger;
import Expressions.*;
import Instructions.*;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;
import static org.junit.jupiter.api.Assertions.*;
import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

public class ProcedureTests {

    private final ByteArrayOutputStream outputStreamCaptor = new ByteArrayOutputStream();

    @BeforeEach
    public void setUp() {
        System.setOut(new PrintStream(outputStreamCaptor));
    }

    // Basic functionality tests and variable manipulation

    @Test
    public void changeOutsideVariableUsingProcedure() {
        Block procedure = new Block.Builder()
                .declareVariable('a', Constant.of(2))
                .declareProcedure(new ProcedureDeclaration.Builder("changea")
                        .assign('a', Constant.of(3))
                        .print(GetVariable.named('a'))
                        .build())
                .print(GetVariable.named('a'))
                .invoke("changea")
                .print(GetVariable.named('a'))
                .build();

        new Debugger(procedure).runDebuggerTest();

        assertEquals("233The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void failChangeOutsideVariableUsingProcedure1() {
        Block procedure = new Block.Builder()
                .declareVariable('a', Constant.of(1))
                .declareProcedure(new ProcedureDeclaration.Builder("changea", 'a')
                        .assign('a', Constant.of(3))
                        .print(GetVariable.named('a'))
                        .build())
                .print(GetVariable.named('a'))
                .invoke("changea", Constant.of(1))
                .print(GetVariable.named('a'))
                .build();

        new Debugger(procedure).runDebuggerTest();

        assertEquals("131The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void failChangeOutsideVariableUsingProcedure2() {
        Block procedure = new Block.Builder()
                .declareVariable('a', Constant.of(3))
                .declareVariable('b', Multiply.of(GetVariable.named('a'), Constant.of(4)))
                .declareVariable('c', Constant.of(0))
                .declareProcedure(new ProcedureDeclaration.Builder("changec", 'a', 'b')
                        .assign('c', Add.of(GetVariable.named('a'), GetVariable.named('b')))
                        .assign('b', Constant.of(0))
                        .assign('a', Constant.of(0))
                        .print(GetVariable.named('c'))
                        .print(GetVariable.named('b'))
                        .print(GetVariable.named('a'))
                        .build())
                .print(GetVariable.named('a'))
                .print(GetVariable.named('b'))
                .print(GetVariable.named('c'))
                .invoke("changec", Constant.of(1), Constant.of(2))
                .print(GetVariable.named('a'))
                .print(GetVariable.named('b'))
                .print(GetVariable.named('c'))
                .build();

        new Debugger(procedure).runDebuggerTest();

        assertEquals("31203003123The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void procedureInvokesAnotherProcedure() {
        Block procedure = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("first", 'a', 'b')
                        .print(GetVariable.named('a'))
                        .print(GetVariable.named('b'))
                        .invoke("second", Constant.of(3), Constant.of(4))
                        .print(GetVariable.named('a'))
                        .print(GetVariable.named('b'))
                        .build())
                .declareProcedure(new ProcedureDeclaration.Builder("second", 'c', 'd')
                        .print(GetVariable.named('a'))
                        .print(GetVariable.named('b'))
                        .assign('a', GetVariable.named('c'))
                        .assign('b', GetVariable.named('d'))
                        .build())
                .invoke("first", Constant.of(1), Constant.of(2))
                .build();

        new Debugger(procedure).runDebuggerTest();

        assertEquals("121234The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void sameName() {
        Block procedure = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("procedure", 'a', 'b')
                        .print(GetVariable.named('a'))
                        .print(GetVariable.named('b'))
                        .build())
                .newBlock(new Block.Builder()
                        .declareProcedure(new ProcedureDeclaration.Builder("procedure", 'a', 'b')
                            .print(Add.of(GetVariable.named('a'), GetVariable.named('b')))
                        .build())
                        .invoke("procedure",  Constant.of(1), Constant.of(2)).build())

                .invoke("procedure", Constant.of(1), Constant.of(2))
                .build();

        new Debugger(procedure).runDebuggerTest();

        assertEquals("312The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    // Error tests

    @Test
    public void wrongName() {
        Block wrongName = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("name", 'a', 'b', 'c')
                        .print(Add.of(Multiply.of(GetVariable.named('a'), GetVariable.named('b')), GetVariable.named('c')))
                        .build())
                .invoke("notname", Constant.of(1), Constant.of(2), Constant.of(3))
                .build();

        new Debugger(wrongName).runDebuggerTest();

        assertEquals("[Exception] This procedure does not exist in this scope!Call procedure called 'notname' with parameters [1, 2, 3].The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void fewerArgumentsThanParameters() {
        Block fewerArguments = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("procedure", 'a', 'b')
                        .print(GetVariable.named('a'))
                        .print(GetVariable.named('b'))
                        .build())
                .invoke("procedure", Constant.of(1))
                .build();

        new Debugger(fewerArguments).runDebuggerTest();

        assertEquals("[Exception] This procedure was called with a different number of arguments than required!Call procedure called 'procedure' with parameters [1].The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void moreArgumentsThanParameters() {
        Block moreArguments = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("procedure", 'a', 'b')
                        .print(GetVariable.named('a'))
                        .print(GetVariable.named('b'))
                        .build())
                .invoke("procedure", Constant.of(1), Constant.of(2), Constant.of(3))
                .build();

        new Debugger(moreArguments).runDebuggerTest();

        assertEquals("[Exception] This procedure was called with a different number of arguments than required!Call procedure called 'procedure' with parameters [1, 2, 3].The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void doubleDeclarationOfProcedures() {
        Block procedure = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("procedure", 'a', 'b')
                        .print(GetVariable.named('a'))
                        .print(GetVariable.named('b'))
                        .invoke("procedure", Constant.of(3), Constant.of(4))
                        .print(GetVariable.named('a'))
                        .print(GetVariable.named('b'))
                        .build())
                .declareProcedure(new ProcedureDeclaration.Builder("procedure", 'c', 'd')
                        .print(GetVariable.named('a'))
                        .print(GetVariable.named('b'))
                        .assign('a', GetVariable.named('c'))
                        .assign('b', GetVariable.named('d'))
                        .build())
                .invoke("procedure", Constant.of(1), Constant.of(2))
                .build();

        new Debugger(procedure).runDebuggerTest();

        assertEquals("[Exception] This procedure has already been declared in this scope!Declare procedure called 'procedure' with parameters [c, d].The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void doubleParameterDeclaration() {
        Block doubleParameters = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("procedure", 'a', 'a')
                        .print(GetVariable.named('a'))
                        .build())
                .invoke("procedure", Constant.of(1), Constant.of(2))
                .build();

        new Debugger(doubleParameters).runDebuggerTest();

        assertEquals("[Exception] This parameter has already been declared in this procedure!Declare procedure called 'procedure' with parameters [a, a].The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    // Other functionality tests

    @Test
    public void recursion() {
        Block recursion = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("recursion", 'x')
                        .assign('x', Subtract.of(GetVariable.named('x'), Constant.of(1)))
                        .print(GetVariable.named('x'))
                        .condition(new IfElseStatement.Builder(
                                GetVariable.named('x'), Condition.GREATER_EQUAL ,Constant.of(1))
                                .invokeIfTrue("recursion",
                                        GetVariable.named('x'))
                                .build())
                        .build())
                .invoke("recursion", Constant.of(10))
                .build();

        new Debugger(recursion).runDebuggerTest();

        assertEquals("9876543210The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void fibonacci() {
        Block fibonacci = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("fibonacci", 'n')
                        .declareVariable('a', Constant.of(0))
                        .declareVariable('b', Constant.of(0))
                        .declareVariable('c', Constant.of(1))
                        .forLoop(new ForLoop.Builder('i', Subtract.of(GetVariable.named('n'), Constant.of(1)))
                                .assign('a', GetVariable.named('b'))
                                .assign('b', GetVariable.named('c'))
                                .assign('c', Add.of(GetVariable.named('a'), GetVariable.named('b'))).build())
                        .print(GetVariable.named('c')).build()
                        )
                .invoke("fibonacci", Constant.of(40))
                .build();

        new Debugger(fibonacci).runDebuggerTest();

        assertEquals("102334155The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }
}
