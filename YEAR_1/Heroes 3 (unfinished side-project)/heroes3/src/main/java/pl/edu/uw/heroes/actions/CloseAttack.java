package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.simulation.GameState;
import pl.edu.uw.heroes.units.Unit;

public class CloseAttack extends Attack {
    public CloseAttack(Unit unit, Unit attackedUnit) {
        super(unit, attackedUnit);
    }

    @Override
    public void execute(GameState gameState) {
        if (unit.getStatistics().isRanged()) {
            attackedUnit.isAttacked(calculateDamage() * 0.5);
        } else {
            attackedUnit.isAttacked(calculateDamage());
        }
        CounterAttack counter = new CounterAttack(attackedUnit, unit);
        counter.execute(gameState);
    }

    @Override
    public String toString() {
        return "Unit " + unit + " close attacks " + attackedUnit;
    }
}
