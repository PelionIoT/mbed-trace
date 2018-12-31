# mbed-os 5 example application using mbed-trace library

## build

```
mbed deploy
mbed compile -t GCC_ARM -m K64F
```


## Usage

When you flash a target with this application and open a terminal you should see the following traces:

```
[INFO][main] Hello tracers
[DBG ][main] Infinite loop..
[DBG ][main] Infinite loop..
[DBG ][main] Infinite loop..
...
```
