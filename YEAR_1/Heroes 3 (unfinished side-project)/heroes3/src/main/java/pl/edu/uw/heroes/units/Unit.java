package pl.edu.uw.heroes.units;

import lombok.Getter;
import lombok.Setter;
import pl.edu.uw.heroes.actions.SpecialAbility;
import pl.edu.uw.heroes.players.Player;
import pl.edu.uw.heroes.board.Field;

import java.util.ArrayList;

public abstract class Unit {

    protected Unit(Player owner, UnitTypes type, Field field, UnitStatistics statistics, Squad squad) {
        this.owner = owner;
        this.speed = statistics.getSpeed();
        this.type = type;
        this.field = field;
        this.statistics = statistics;
        this.squad = squad;
        this.defense = statistics.getDefense();
        this.health = statistics.getHealth();
    }

    protected Player owner;

    @Getter
    protected ArrayList<SpecialAbility> specialAbilities = new ArrayList<>();

    protected int speed;

    protected UnitTypes type;

    protected Field field;

    protected UnitStatistics statistics;

    @Setter
    @Getter
    protected Squad squad;

    @Getter
    protected double defense;

    protected double health;

    @Getter
    private boolean dead = false;

    protected boolean hasWaitedInThisRound = false;

    @Getter
    protected boolean hasCounterAttackedInThisRound = false;

    public Player getOwner() {
        return owner;
    }

    public int getSpeed() {
        return speed;
    }

    public boolean canWait() {
        return !hasWaitedInThisRound;
    }

    public void doWait() {
        hasWaitedInThisRound = true;
    }

    public abstract void doDefend();

    public void doMove(Field destination) {
        if (field != null)
            field.setUnit(null);
        field = destination;
        destination.setUnit(this);
    }

    public void resetRound() {
        hasWaitedInThisRound = false;
        hasCounterAttackedInThisRound = false;
        this.defense = statistics.getDefense();
    }

    public UnitStatistics getStatistics() {
        return statistics;
    }

    public boolean isFlying() {
        return statistics.isFlying();
    }

    public Field getField() {
        return field;
    }

    public UnitTypes getType() {
        return type;
    }

    public void isAttacked(double damage) {
        this.health -= damage;

        if (this.health <= 0) {
            this.dead = true;
            this.getOwner().getUnits().remove(this);
            squad.isAttacked(Math.abs(health));
        }
    }
    public Squad getSquad() {
        return squad;
    }

    public double getDefense() {
        return defense;
    }
}
