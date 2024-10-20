declare namespace types {
    export interface GameState {
        card_hand: CardHand;
        next_card: Card;
        current_player: Player;
        opponent: Player;
        arena: Arena;
    }

    interface Player {
        name: string;
        elixir_count: Elixir;
        left_tower: Tower;
        right_tower: Tower;
    }

    interface Tower {
        health: number;
    }

    interface Arena {
        units: Array<Unit>;
    }

    interface Unit {
        team: Team;
        health: number;
    }

    enum Team {
        RED = "RED",
        BLUE = "BLUE",
    }

    interface CardHand {
        card_1: Card;
        card_2: Card;
        card_3: Card;
        card_4: Card;
    }

    interface Card {
        name: string;
        elixir_cost: Elixir;
    }

    interface ActionCard {
        card_name: string;
        tile_position: { x: number, y: number };
    }

    type Elixir = 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10;

    type RunCallback = (state: GameState) => ActionCard | void;

    /** @customConstructor cr.Bot.new */
    export class Bot {
        constructor();
        step(callback: RunCallback): void;
    }
}