package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.simulation.GameState;
import pl.edu.uw.heroes.units.Unit;

public abstract class Attack extends UnitAction {
    public Attack(Unit unit, Unit attackedUnit) {
        super(unit);
        this.attackedUnit = attackedUnit;
    }

    protected final Unit attackedUnit;

    private int getRandomNumber(int min, int max) {
        return (int) ((Math.random() * (max - min)) + min);
    }

    protected double calculateDamage() {
        double sum = 0;

        double baseDamage = 0;

        if (unit.getSquad().getNumberOfMembers() < 10) {
            for (Unit u: unit.getSquad().getSquad()) {
                baseDamage += getRandomNumber((int) unit.getStatistics().getDamageMin(), (int) unit.getStatistics().getDamageMax());
            }
        } else {
            for (int i = 0; i < 10; i++) {
                baseDamage += getRandomNumber((int) unit.getType().getDamageMin(), (int) unit.getType().getDamageMax());
            }

            baseDamage *= Math.floor((double) unit.getSquad().getNumberOfMembers() / 10);
        }

        double damageBonus = 1;

        if (unit.getStatistics().getAttack() > attackedUnit.getStatistics().getAttack() + 10) {
            damageBonus += 0.5;
        }

        double damageReduction = 1;

        if (attackedUnit.getDefense() > unit.getStatistics().getAttack()) {
            damageReduction *= 1 - (0.025 * attackedUnit.getDefense() - unit.getStatistics().getAttack());
        }

        return baseDamage * damageReduction * damageBonus;
    }

    public abstract void execute(GameState gameState);
}
