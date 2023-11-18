package Instructions;
import Debugger.*;

public abstract class Instruction {
    public abstract void execute(Debugger debugger) throws Exception;

    public abstract void printName();
}
