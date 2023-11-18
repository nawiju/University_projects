package pl.edu.uw.heroes.simulation;

import lombok.Value;
import pl.edu.uw.heroes.players.Player;
import pl.edu.uw.heroes.actions.Action;
import pl.edu.uw.heroes.board.Board;
import pl.edu.uw.heroes.units.Unit;

import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

@Value
public class GameState {

    Player playerLeft, playerRight;

    Board board;

    List<Action> executedActions = new LinkedList<>();

    Queue<Unit> units = new LinkedList<>();
}
