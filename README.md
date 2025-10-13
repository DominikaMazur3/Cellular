# Cellular
C++ | Raylib | cellular automata  
🚧 Development started: Jul 30, 2025  
🌟 Features: simulating user-defined rules for cellular automata, updating cells manually or automatically, the ability to reload the rules while the program is running  

## Defining rules
Rules need to be defined in `rules.txt` file in the same directory as the executable. Each rule should be in a new line.  
Currently only one rule type is supported:  
`x y`  
It means if the cell has x neighbours which value is 1 change its state to y. For example a line containing ONLY `5 0` would be interpreted as if a cell has 5 neighbours which value is 1, change its state to 0.  
You can also use `mode 4` or `mode 8` in its own separate line to specify whether 4 or 8 neighbours should be considered (TOP-LEFT-RIGHT-BOTTOM or a 3x3 square without the middle cell).
