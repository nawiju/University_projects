package pl.edu.uw.heroes.units;

import lombok.ToString;
import pl.edu.uw.heroes.players.Player;

import static pl.edu.uw.heroes.units.UnitTypes.*;

@ToString
public class Unicorn extends Unit {
    public Unicorn(Player owner) {
        super(owner, UNICORN, null, new UnitStatistics(false, 7, 15, 14, 0, UNICORN.getDamageMin(), UNICORN.getDamageMax(), 90, false, false), null);
    }

    @Override
    public void doDefend() {
        this.defense = this.statistics.getDefense() * 1.2;
    }
}
