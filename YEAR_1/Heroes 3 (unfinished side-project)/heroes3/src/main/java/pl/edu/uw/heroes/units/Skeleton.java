package pl.edu.uw.heroes.units;

import lombok.ToString;
import pl.edu.uw.heroes.players.Player;

import static pl.edu.uw.heroes.units.UnitTypes.*;

@ToString
public class Skeleton extends Unit {

    public Skeleton(Player owner) {
        super(owner, SKELETON, null, new UnitStatistics(false, 4, 5, 4, 0, SKELETON.getDamageMin(), SKELETON.getDamageMax(), 6, false, false), null);
    }

    @Override
    public void doDefend() {
        this.defense = this.statistics.getDefense() * 1.12;
    }
}
