function main() {
    const bot = new types.Bot();
    let last_run_time = 0;
    while (true) {
        const now = tick();
        if (now - last_run_time < 100) {
            continue;
        }
        last_run_time = now;
        bot.step((game_state: types.GameState) => {
            return {
                card_name: "",
                tile_position: { x: 1, y: 1 }
            }
        });
    }
}

main();