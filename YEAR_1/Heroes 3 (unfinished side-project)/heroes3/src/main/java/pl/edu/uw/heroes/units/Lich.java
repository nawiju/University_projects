package pl.edu.uw.heroes.units;

import lombok.ToString;
import pl.edu.uw.heroes.actions.SpecialAbility;
import pl.edu.uw.heroes.players.Player;

import static pl.edu.uw.heroes.units.UnitTypes.LICH;

@ToString
public class Lich extends Unit {

    public Lich(Player owner) {
        super(owner, LICH, null, new UnitStatistics(false, 6, 13, 10, 12, LICH.getDamageMin(), LICH.getDamageMax(), 30, true, true), null);
        specialAbilities.add(SpecialAbility.DEATH_CLOUD);
    }

    @Override
    public void doDefend() {
        this.defense = this.statistics.getDefense() * 1.12;
    }
}
