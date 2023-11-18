package pl.edu.uw.heroes.players;

import lombok.Getter;
import pl.edu.uw.heroes.actions.Action;
import pl.edu.uw.heroes.units.Squad;
import pl.edu.uw.heroes.units.Unit;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Random;

public class ExamplePlayer implements Player {

    private final String NO_SPACE = "[Exception] No space for new unit.";
    private final String NO_UNIT = "[Exception] Out of bounds.";

    private final String name;

    public ExamplePlayer(String name) {
        this.name = name;
    }

    @Getter
    private final Collection<Unit> units = new ArrayList<>();

    private final Random random = new Random();

    @Getter
    private Squad[] squad = new Squad[20];

    private int squadTop = 0;

    @Getter
    private int numberOfUnits = 0;

    @Override
    public void addToPlayerUnits(Unit unit) throws Exception {
        addToSquad(unit);
        units.add(unit);
    }

    @Override
    public Unit getUnitFromSquad(int index) throws Exception {
        if (index < 0 || index > 19) {
            return null;
        } else if (squad[index] != null) {
                return squad[index].squadPeek();
        } else {
            return null;
        }
    }

    public void addToSquad(Unit unit) throws Exception {
        if (squadTop < 20) {
            squad[squadTop++] = new Squad();
            squad[squadTop - 1].addToSquad(unit);
            unit.setSquad(squad[squadTop - 1]);
        } else {
            boolean found = false;
            for (Squad s: squad) {
                if (s.getType() == unit.getType()) {
                    s.addToSquad(unit);
                    found = true;
                    break;
                }
            }

            if (!found) {
                throw new Exception(NO_SPACE);
            }
        }
    }

    @Override
    public Action chooseAction(Collection<Action> actions) {
        //  System.out.println("Available actions for " + name + ": ");
        //  actions.forEach(System.out::println);
        return actions.stream()
                .skip(random.nextInt(actions.size()))
                .findFirst()
                .get();
    }

    @Override
    public String toString() {
        return name;
    }
}
