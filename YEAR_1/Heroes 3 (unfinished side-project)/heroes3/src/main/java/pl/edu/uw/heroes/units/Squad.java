package pl.edu.uw.heroes.units;

import lombok.Getter;

import java.util.LinkedList;
import java.util.Queue;

public class Squad {

    private final String INVALID_UNIT_TYPE = "[Exception] Invalid unit type.";

    @Getter
    private Queue<Unit> squad;

    public Squad() {
        squad = new LinkedList<Unit>();
    }

    @Getter
    private UnitTypes type = null;

    @Getter
    private int numberOfMembers = 0;

    public void addToSquad(Unit unit) throws Exception {
        if(type == null) {
            type = unit.getType();
            squad.add(unit);
            numberOfMembers++;
        } else if (type == unit.getType()) {
            squad.add(unit);
            numberOfMembers++;
        } else {
            throw new Exception(INVALID_UNIT_TYPE);
        }
    }

    public void removeFromSquad() {
        Unit deceasedUnit = squad.poll();
        numberOfMembers--;
        if (numberOfMembers > 0) {
            deceasedUnit.getField().setUnit(squad.peek());
        } else {
            deceasedUnit.getField().setUnit(null);
            type = null;
        }
    }

    public void isAttacked(double damage) {
        if (damage > 0) {
            removeFromSquad();
            if (numberOfMembers > 0) {
                squad.peek().isAttacked(damage);
            }
        }
    }

    public Unit squadPeek() {
        return squad.peek();
    }
}
