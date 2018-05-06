# mbed-os 5 example application using mbed-trace library

## build

```
mbed deploy
mbed compile -t GCC_ARM -m K64F
```


## Usage

When you flash target with this application and open terminal you should see following traces:

```
[INFO][main] Hello tracers
[DBG ][main] Infinite loop..
[DBG ][main] Infinite loop..
[DBG ][main] Infinite loop..
...
```
