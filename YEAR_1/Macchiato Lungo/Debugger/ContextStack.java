package Debugger;

import java.util.Arrays;

public class ContextStack {

    private final String TOO_FEW_LEVELS = "[Exception] There are fewer levels than the number inputted.";

    private BlockContext[] contexts = new BlockContext[10];

    private int topIndex = 0;

    public void push(BlockContext context) {
        if (topIndex == contexts.length) {
            contexts = Arrays.copyOf(contexts, contexts.length * 2);
        }
        contexts[topIndex++] = context;
    }

    public BlockContext peek() {
        if (topIndex > 0) {
            return contexts[topIndex - 1];
        }
        return null;
    }

    public BlockContext fetch(int n) throws Exception {
        if (topIndex - n > 0) {
            int i = topIndex - 1;

            for (int j = 0; j < n; j++) {
                i--;
            }

            return contexts[i];
        } else {
            throw new Exception(TOO_FEW_LEVELS);
        }
    }

    public BlockContext pop() throws Exception {
        if (topIndex > 0) {
            return contexts[--topIndex];
        } else {
            throw new Exception(TOO_FEW_LEVELS);
        }
    }
}
