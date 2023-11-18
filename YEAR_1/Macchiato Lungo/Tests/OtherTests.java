package Tests;
import Debugger.Debugger;
import Expressions.*;
import Instructions.*;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

import static org.junit.jupiter.api.Assertions.*;

/**
 * These tests will test every single instruction as to fulfill the changing project specifications.
 */

public class OtherTests {
    private final ByteArrayOutputStream outputStreamCaptor = new ByteArrayOutputStream();

    @BeforeEach
    public void setUp() {
        System.setOut(new PrintStream(outputStreamCaptor));
    }

    /**
     * The follwing tests will test the functionality of expressions.
     */
    @Test
    public void addition() {
        Block addition = new Block.Builder()
                .print(Add.of(Constant.of(5), Constant.of(2)))
                .build();

        new Debugger(addition).runDebuggerTest();

        assertEquals("7The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void subtraction() {
        Block subtraction = new Block.Builder()
                .print(Subtract.of(Constant.of(5), Constant.of(2)))
                .build();

        new Debugger(subtraction).runDebuggerTest();

        assertEquals("3The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void modulo() {
        Block modulo = new Block.Builder()
                .print(Modulo.of(Constant.of(5), Constant.of(3)))
                .build();

        new Debugger(modulo).runDebuggerTest();

        assertEquals("2The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void division() {
        Block division = new Block.Builder()
                .print(Divide.of(Constant.of(5), Constant.of(2)))
                .build();

        new Debugger(division).runDebuggerTest();

        assertEquals("2The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void multiplication() {
        Block multiplication = new Block.Builder()
                .print(Multiply.of(Constant.of(5), Constant.of(2)))
                .build();

        new Debugger(multiplication).runDebuggerTest();

        assertEquals("10The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void getVariable() {
        Block getVariable = new Block.Builder()
                .declareVariable('a', Constant.of(9))
                .print(GetVariable.named('a'))
                .build();

        new Debugger(getVariable).runDebuggerTest();

        assertEquals("9The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    // Other instructions.
    // Please keep in mind that some instructions have already been tested in other files (ProcedureTests and BuilderTests) therefore adding them here would be superfluous and unnecessary.

    @Test
    public void assignVariable() {
        Block assignVariable = new Block.Builder()
                .declareVariable('a', Constant.of(6))
                .print(GetVariable.named('a'))
                .assign('a', Constant.of(9))
                .print((GetVariable.named('a')))
                .build();

        new Debugger(assignVariable).runDebuggerTest();

        assertEquals("69The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void procedureDeclarationAndInvoke() {
        Block procedureDeclarationAndInvoke = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("metallicaisthebest")
                        .print(Constant.of(666))
                        .build())
                .invoke("metallicaisthebest")
                .build();

        new Debugger(procedureDeclarationAndInvoke).runDebuggerTest();

        assertEquals("666The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void procedureProcedureDeclarationAndInvoke() {
        Block procedureProcedureDeclarationAndInvoke = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("metallicaisthebest")
                        .declareVariable('a', Constant.of(70))
                        .declareProcedure(new ProcedureDeclaration.Builder("essa", 'e')
                                .print(Subtract.of(Constant.of(70), GetVariable.named('e')))
                                .build())
                        .invoke("essa", Constant.of(1))
                        .build())
                .invoke("metallicaisthebest")
                .build();

        new Debugger(procedureProcedureDeclarationAndInvoke).runDebuggerTest();

        assertEquals("69The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void procedureVariableDeclarationAndUse() {
        Block procedureVariableDeclarationAndUse = new Block.Builder()
                .declareProcedure(new ProcedureDeclaration.Builder("slipknotisamazing")
                        .declareVariable('a', Constant.of(555))
                        .print((GetVariable.named('a')))
                        .print(Add.of(GetVariable.named('a'), Constant.of(111)))
                        .build())
                .invoke("slipknotisamazing")
                .build();

        new Debugger(procedureVariableDeclarationAndUse).runDebuggerTest();

        assertEquals("555666The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void declareVariable() {
        Block declareVariable = new Block.Builder()
                .declareVariable('n', Constant.of(666))
                .print(GetVariable.named('n'))
                .build();

        new Debugger(declareVariable).runDebuggerTest();

        assertEquals("666The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void printTest() {
        Block printTest = new Block.Builder()
                .print(Constant.of(420))
                .build();

        new Debugger(printTest).runDebuggerTest();

        assertEquals("420The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }
}
