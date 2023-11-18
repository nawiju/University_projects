package Tests;
import Debugger.Debugger;
import Expressions.*;
import Instructions.*;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;
import static org.junit.jupiter.api.Assertions.*;
import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

public class BuilderTests {
    private final ByteArrayOutputStream outputStreamCaptor = new ByteArrayOutputStream();

    @BeforeEach
    public void setUp() {
        System.setOut(new PrintStream(outputStreamCaptor));
    }

    @Test
    public void forLoop() {
        Block program = new Block.Builder()
                .declareVariable('n', Constant.of(10))
                .forLoop(new ForLoop.Builder('i', GetVariable.named('n'))
                        .print(GetVariable.named('i'))
                        .assign('i', Add.of(Constant.of(1), GetVariable.named('i')))
                        .print(GetVariable.named('i'))
                        .build())
                .build();

        new Debugger(program).runDebuggerTest();

        assertEquals("011223344556677889910The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void testBlock() {
        Block program = new Block.Builder()
                .declareVariable('n', Constant.of(10))
                .newBlock(new Block.Builder()
                        .declareVariable('m', Constant.of(59))
                        .print(Add.of(GetVariable.named('m'), GetVariable.named('n')))
                        .build())
                .build();

        new Debugger(program).runDebuggerTest();

        assertEquals("69The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }

    @Test
    public void testIfElseStatement1() {
            Block program = new Block.Builder()
                    .declareVariable('n', Constant.of(10))
                    .condition(new IfElseStatement.Builder(Modulo.of(GetVariable.named('n'), Constant.of(2)), Condition.EQUALS, Constant.of(0))
                            .printIfTrue(Constant.of(420))
                            .printIfFalse(Constant.of(2137))
                            .build())
                    .build();

            new Debugger(program).runDebuggerTest();

            assertEquals("420The program has finished running.", outputStreamCaptor
                    .toString().trim().replace("\n", "")
                    .replace("\r", ""));
    }

    @Test
    public void testIfElseStatement2() {
        Block program = new Block.Builder()
                .declareVariable('n', Constant.of(11))
                .condition(new IfElseStatement.Builder(Modulo.of(GetVariable.named('n'), Constant.of(2)), Condition.EQUALS, Constant.of(0))
                        .printIfTrue(Constant.of(420))
                        .printIfFalse(Constant.of(2137))
                        .build())
                .build();

        new Debugger(program).runDebuggerTest();

        assertEquals("2137The program has finished running.", outputStreamCaptor
                .toString().trim().replace("\n", "")
                .replace("\r", ""));
    }
}
