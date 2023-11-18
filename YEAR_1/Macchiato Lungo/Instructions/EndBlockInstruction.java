package Instructions;

import Debugger.Debugger;

public class EndBlockInstruction extends Instruction {

    public EndBlockInstruction() {}

    @Override
    public void printName() {
        System.out.println("End block.");
    }

    @Override
    public void execute(Debugger debugger) throws Exception {
        try {
            debugger.removeTopContext();
        } catch (Exception e) {
            throw e;
        }
    }
}
