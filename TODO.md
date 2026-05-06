config files for ports indicating what cli options they use/support, what file types they support, and what complevels

file cli options examples:

| extension | cli option|
| - | - |
| **Odamex** | |
|wad| -file |
|deh| -deh |
|bex| -bex |
|pk3/pk7| unsupported |
|**Chocolate** | |
|wad| -merge |
|deh| -deh |
|bex| -deh |
|pk3| unsupported |
|**Woof**| |
|wad| -file |
|deh| -deh |
|bex| -deh |
|pk3/pk7| unsupported-ish |
|**UZDoom** | |
|wad| -file |
|deh| -deh |
|bex| -bex |
|pk3| -file |
|pk7| -file |

not sure yet if we should just implicitly have file type support in the cli options or if we should also have a separate list of supported file types. probably the latter

should chocolates -file also be added as an option somehow? no other port has the distinction between the two ways of loading

Common cli options:
| effect | cli option |
| - | - |
| no monsters | -nomonsters |
| fast monsters | -fastmonsters |
| pistol start | -pistolstart (todo: check if any use -pistol) |
| respawn monsters | -respawn |
| coop things | -coop-things or -coop_spawns |

uzdoom might need something like `+sv_pistolstart 1`?
also need to research if it has support for anything like `-coop-things`

These should be able to be toggle per session as well as part of presets, when used in presets, they should be able to be filtered if the currently selected port does not support them. Port configs will list what that ports specific cli options are for each feature. Lack of support will just be indicated by the value for the option rather than a separate list.

also need a way to define additional cli options that are either globally applied to a port or as part of a preset. things like nodeh, nomapinfo, etc are too specific to be included in the menus.

**complevels**

not sure what to do for these, want to be able to present nice names like woof uses, but need to make sure to have a way of selecting the weird ones that only have numbers too. need to check if theres any port that supports only the names or if it works to supply just the numbers in all cases. simpler for supporting both woof and dsda, and the ui can still use nice names (probably have an option that limits the complevels displayed to just the common ones or display all of them), number should be in parentheses after the name for clarity, e.g. Boom (CL9), MBF (CL11)

uzdoom has `-compatmode` with these values:

| value | description |
| - | - |
| 0 | default |
| 1 | doom |
| 2 | doom (strict) |
| 3 | boom |
| 6 | boom (strict) |
| 5 | mbf |
| 7 | mbf (strict) |
| 8 | mbf21 |
| 9 | mbf21 (strict) |
| 4 | doom 2.063 |

this is a persistent change however

for both this and sv_pistolstart, we want to see if theres a way to do these temporarily instead of overwriting the users settings

**file types**
when adding individual wads to the library, we should have extension filters

the extensions i know of are wad, deh, bex, pk3, pk7, pke, zip, 7z, pkz, ipk3, ipk7, pwad, iwad
we want the all files option to always be available, and if possible we probably want multiple sets of filters, its simpler to have some of these just in the drop down
