package pl.edu.uw.heroes.units;

import lombok.ToString;
import pl.edu.uw.heroes.actions.SpecialAbility;
import pl.edu.uw.heroes.players.Player;

import static pl.edu.uw.heroes.units.UnitTypes.DRAGON;

@ToString
public class Dragon extends Unit {

    public Dragon(Player owner) {
        super(owner, DRAGON, null, new UnitStatistics(true, 16, 27, 27, 0, DRAGON.getDamageMin(), DRAGON.getDamageMax(), 250, true, true), null);
        specialAbilities.add(SpecialAbility.DRAGON_BREATH);
    }

    @Override
    public void doDefend() {
        this.defense = this.statistics.getDefense() * 1.12;
    }
}
