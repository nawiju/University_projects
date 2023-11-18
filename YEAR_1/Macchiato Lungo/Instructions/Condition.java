package Instructions;

public enum Condition {
    EQUALS, NOT_EQUAL, LESS_THAN, GREATER_THAN, SMALLER_EQUAL, GREATER_EQUAL;

    @Override
    public String toString() {
        switch(this) {
            case EQUALS -> {
                return "=";
            }
            case NOT_EQUAL -> {
                return "<>";
            }
            case LESS_THAN -> {
                return "<";
            }
            case GREATER_THAN -> {
                return ">";
            }
            case SMALLER_EQUAL -> {
                return "<=";
            }
            case GREATER_EQUAL -> {
                return ">=";
            }
        }
        return null;
    }
}