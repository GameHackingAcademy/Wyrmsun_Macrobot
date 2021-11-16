# Wyrmsun Macrobot
Referenced in https://gamehacking.academy/lesson/4/3.

A hack for Wyrmsun version 5.0.1 that will automatically create worker units out of the currently selected structure when a player's gold is over 3000.

It accomplishes this by filling the current unit buffer with worker data and then calling the create unit function in the game.

After injecting this hack, go in game and recruit a worker. Then select a structure as you collect gold. You will notice workers being queued automatically. Due to the way Wyrmsun handles recruitment, it is possible to create units out of whatever is selected, including other units.

This must be injected into the Wyrmsun process to work. One way to do this is to use a DLL injector. Another way is to enable AppInit_DLLs in the registry.
