package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.simulation.GameState;
import pl.edu.uw.heroes.units.Unit;

public class Defend extends UnitAction {

    public Defend(Unit unit) {
        super(unit);
    }

    @Override
    public void execute(GameState gameState) {
        unit.doDefend();
    }

    @Override
    public String toString() {
        return "Unit " + unit + " defends itself";
    }
}
