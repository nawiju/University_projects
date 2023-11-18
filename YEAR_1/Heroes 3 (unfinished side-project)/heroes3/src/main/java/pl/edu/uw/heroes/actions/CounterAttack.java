package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.simulation.GameState;
import pl.edu.uw.heroes.units.Unit;

public class CounterAttack extends Attack {

    public CounterAttack(Unit unit, Unit attackedUnit) {
        super(unit, attackedUnit);
    }

    @Override
    public void execute(GameState gameState) {
        if (unit != null && !unit.isHasCounterAttackedInThisRound() && !unit.isDead()) {
            attackedUnit.isAttacked(calculateDamage());
            System.out.println(this.toString());
        }
    }

    @Override
    public String toString() {
        return "Unit " + unit + " counter-attacks " + attackedUnit;
    }
}
