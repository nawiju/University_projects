package pl.edu.uw.heroes.actions;

import pl.edu.uw.heroes.simulation.GameState;

public interface Action {

    void execute(GameState gameState);
}
