package Tests;
import Debugger.Debugger;
import Expressions.*;
import Instructions.*;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;
import static org.junit.jupiter.api.Assertions.*;

import java.io.*;

public class FileDumpTests {
    private final ByteArrayOutputStream outputStreamCaptor = new ByteArrayOutputStream();

    @BeforeEach
    public void setUp() {
        System.setOut(new PrintStream(outputStreamCaptor));
    }

    @Test
    public void fileDumpTestOneScope() {
        Block program = new Block.Builder()
                .declareVariable('a', Constant.of(6))
                .declareVariable('b', Constant.of(9))
                .declareProcedure(new ProcedureDeclaration.Builder("hello", 'c', 'd', 'e').assign('a', Constant.of(420)).build())
                .invoke("hello", Constant.of(4), Constant.of(2), Constant.of(0))
                .build();

        try {
            Debugger debugger = new Debugger(program);
            debugger.steps(4);
            debugger.dump("a.in");

            File a = new File("a.in");
            assertTrue(a.exists() && !a.isDirectory());

            debugger.steps(2);
            debugger.dump("b.in");

            File b = new File("b.in");
            assertTrue(b.exists() && !b.isDirectory());

            debugger.steps(10);

            String[] answerA = {"Variables in scope:", "a 6", "b 9", "Procedures in scope:", "hello"};
            String[] answerB = {"Variables in scope:", "a 420", "b 9", "c 4", "d 2", "e 0", "Procedures in scope:", "hello"};

            BufferedReader readerA;
            readerA = new BufferedReader(new FileReader("a.in"));

            for (int i = 0; i < answerA.length; i++) {
                String line = readerA.readLine();
                assertTrue(line.trim().replace("\n", "")
                        .replace("\r", "").equals(answerA[i]));
            }

            assertTrue(readerA.readLine() == null);

            BufferedReader readerB;
            readerB = new BufferedReader(new FileReader("b.in"));

            for (int i = 0; i < answerB.length; i++) {
                String line = readerB.readLine();
                assertTrue(line.trim().replace("\n", "")
                        .replace("\r", "").equals(answerB[i]));
            }

            assertTrue(readerB.readLine() == null);

            readerB.close();
            readerA.close();

            a.delete();
            b.delete();
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
    }

    @Test
    public void fileDumpTestTwoScopes() {
        Block program = new Block.Builder()
                .declareVariable('a', Constant.of(1))
                .declareVariable('b', Constant.of(0))
                .declareProcedure(new ProcedureDeclaration.Builder("aproc", 'c', 'd').assign('a', Constant.of(11)).build())
                .declareProcedure(new ProcedureDeclaration.Builder("bproc", 'c', 'd')
                        .declareVariable('e', Constant.of(3))
                        .assign('a', Constant.of(11)).build())
                .invoke("aproc", Constant.of(6), Constant.of(9))
                .invoke("bproc", Constant.of(6), Constant.of(9))
                .build();

        try {
            Debugger debugger = new Debugger(program);
            debugger.steps(6);
            debugger.dump("a.in");

            File a = new File("a.in");
            assertTrue(a.exists() && !a.isDirectory());

            debugger.steps(4);
            debugger.dump("b.in");

            File b = new File("b.in");
            assertTrue(b.exists() && !b.isDirectory());

            debugger.steps(5);

            String[] answerA = {"Variables in scope:", "a 1", "b 0", "c 6", "d 9", "Procedures in scope:", "bproc", "aproc"};
            String[] answerB = {"Variables in scope:", "a 11", "b 0", "c 6", "d 9", "e 3", "Procedures in scope:", "bproc", "aproc"};

            BufferedReader readerA;
            readerA = new BufferedReader(new FileReader("a.in"));

            for (int i = 0; i < answerA.length; i++) {
                String line = readerA.readLine();
                assertTrue(line.trim().replace("\n", "")
                        .replace("\r", "").equals(answerA[i]));
            }

            assertTrue(readerA.readLine() == null);

            BufferedReader readerB;
            readerB = new BufferedReader(new FileReader("b.in"));

            for (int i = 0; i < answerB.length; i++) {
                String line = readerB.readLine();
                assertTrue(line.trim().replace("\n", "")
                        .replace("\r", "").equals(answerB[i]));
            }

            assertTrue(readerB.readLine() == null);

            readerB.close();
            readerA.close();

            a.delete();
            b.delete();
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
    }

    @Test
    public void NoVariablesOrProcedures() {
        Block program = new Block.Builder()
                .print(Constant.of(0))
                .build();

        try {
            Debugger debugger = new Debugger(program);
            debugger.steps(2);
            debugger.dump("no.in");

            File no = new File("no.in");
            assertTrue(no.exists() && !no.isDirectory());

            debugger.steps(2);

            String[] answerNO = {"Variables in scope:", "Procedures in scope:"};

            BufferedReader readerNO;
            readerNO = new BufferedReader(new FileReader("no.in"));

            for (int i = 0; i < answerNO.length; i++) {
                String line = readerNO.readLine();
                assertTrue(line.trim().replace("\n", "")
                        .replace("\r", "").equals(answerNO[i]));
            }

            assertTrue(readerNO.readLine() == null);

            readerNO.close();
            no.delete();

        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
    }
}