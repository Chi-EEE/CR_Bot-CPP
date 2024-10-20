function coroutine_main() {
    const coroutines = [
        coroutine.create(() => {
            const bot = new types.Bot();
            let last_run_time = 0;
            while (true) {
                const now = tick();
                if (now - last_run_time < 100) {
                    coroutine.yield();
                    continue;
                }
                last_run_time = now;
                bot.step((game_state: types.GameState) => {
                    return {
                        card_name: "",
                        tile_position: { x: 1, y: 1 }
                    }
                });
                coroutine.yield();
            }
        }),
        coroutine.create(() => {
            const bot = new types.Bot();
            let last_run_time = 0;
            while (true) {
                const now = tick();
                if (now - last_run_time < 100) {
                    coroutine.yield();
                    continue;
                }
                last_run_time = now;
                bot.step((game_state: types.GameState) => {
                    return {
                        card_name: "",
                        tile_position: { x: 2, y: 2 }
                    }
                });
                coroutine.yield();
            }
        })
    ]
    while (true) {
        for (const c of coroutines) {
            coroutine.resume(c)
        }
    } 
}

coroutine_main();