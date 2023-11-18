package pl.edu.uw.heroes.actions;


import pl.edu.uw.heroes.simulation.GameState;
import pl.edu.uw.heroes.units.Unit;

public class Wait extends UnitAction {

    public Wait(Unit unit) {
        super(unit);
    }

    @Override
    public void execute(GameState gameState) {
        if (!unit.canWait())
            throw new IllegalStateException("Unit has already waited this round");
        unit.doWait();
        gameState.getUnits().add(unit);
    }

    @Override
    public String toString() {
        return "Unit " + unit + " waits";
    }
}
