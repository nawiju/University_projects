package pl.edu.uw.heroes;

import pl.edu.uw.heroes.board.Board;
import pl.edu.uw.heroes.board.Position;
import pl.edu.uw.heroes.players.*;
import pl.edu.uw.heroes.simulation.*;
import pl.edu.uw.heroes.units.*;

/**
 * Implementacja ma:
 *      -> statystyki jednostek
 *      -> oddziały
 *      -> algorytm liczenia obrażeń
 *      -> możliwość zaatakowania jednostki przeciwnika wręcz
 *      -> 4-5 przykładowych jednostek ze statystykami z gry
 *      -> stan początkowy gry inicjowany statycznie
 *      -> gracze wykonują albo losowe ruchy
 *      -> gra kończy się w momencie, w którym na planszy zostaną jednostki tylko jednego gracza
 *      -> kontratak
 *      -> możliwość strzału
 *      -> jednostki atakujące obszarowo (Lich i Dragon)
 *
 * Wszystko zostało zaimplementowane do poziomu mojego zrozumienia.
 */

public class Main {

    public static void main(String[] args) throws Exception {
        Player playerLeft = new ExamplePlayer("Adam");
        playerLeft.addToPlayerUnits(new RoyalGriffin(playerLeft));
        playerLeft.addToPlayerUnits(new Dragon(playerLeft));
        playerLeft.addToPlayerUnits(new Skeleton(playerLeft));
        playerLeft.addToPlayerUnits(new RoyalGriffin(playerLeft));
        playerLeft.addToPlayerUnits(new Lich(playerLeft));

        Player playerRight = new ExamplePlayer("Wiktor");
        playerRight.addToPlayerUnits(new Unicorn(playerRight));
        playerRight.addToPlayerUnits(new Unicorn(playerRight));
        playerRight.addToPlayerUnits(new Skeleton(playerRight));
        playerRight.addToPlayerUnits(new Skeleton(playerRight));
        playerRight.addToPlayerUnits(new Skeleton(playerRight));
        playerRight.addToPlayerUnits(new Skeleton(playerRight));
        playerRight.addToPlayerUnits(new SilverPegasus(playerRight));
        playerRight.addToPlayerUnits(new Skeleton(playerRight));
        playerRight.addToPlayerUnits(new Skeleton(playerRight));

        GameState gameState = new GameState(playerLeft, playerRight, new Board(10, 10));

        GameSimulation gameSimulation = new GameSimulation(gameState);

        for (int i = 0; i < Math.max(gameState.getPlayerRight().getUnits().size(), gameState.getPlayerLeft().getUnits().size()); i++) {
            if (playerLeft.getUnitFromSquad(i) != null) {
                playerLeft.getUnitFromSquad(i).doMove(gameState.getBoard().getField(new Position(i, 0)));
            }

            if (playerRight.getUnitFromSquad(i) != null) {
                playerRight.getUnitFromSquad(i).doMove(gameState.getBoard().getField(new Position(i, 9)));
            }
        }


        while(gameState.getPlayerRight().getUnits().size() > 0 && gameState.getPlayerLeft().getUnits().size() > 0) {
            gameSimulation.executeOneAction();
            gameSimulation.printState();
        }
    }
}
