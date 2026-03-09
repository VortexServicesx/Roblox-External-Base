# kjs Roblox External Base ( by kjs.lol on discord!)



## features

- box esp
- name esp (display name / username)
- health bar esp
- rig type esp
- color customization for all of the above

## building



requires visual studio 2022 or 2026 with c++ installed
note; if you encounter the error The build tools for v145 cannot be found. Install v145 to build using the v145 build tools., go to project > properties > platform toolset and set it to v143
```

open kjs.sln

build > release x64

```



output: `build/kjs.exe`

## usage

1. launch roblox

2. run kjs.exe

3. press F1 to toggle menu (change this toggle key in menu.cpp)

## controls

- F1: toggle menu


## notes


- runs at monitor refresh rate (vsync)

- excludes local player from esp

- updates player list every 750ms (just about right in my opinion)

- fairly low cpu usage i would say



## structure



```

src/
 cache/       - player caching
 hacks/       - esp rendering
 memory/      - external memory reading
 menu/        - interface
 sdk/         - roblox instances
 utils/       - offsets, math, globals, keybinds

deps/
 imgui/       - imgui library dependencies

```



# so yea uh hope you enjoy and message me on discord if you find any bugs/issues or you added anything and would like to contribute!



