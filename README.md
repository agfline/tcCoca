
# LibTC

LibTC is a C-coded library for SMPTE / EBU timecode handling, display, conversion and calculation.

It is based upon the **SMPTE ST 12-1** standard (formally SMPTE 12M) and the **SMPTE ST 12-3** for HFR. Those define the following rates : 23.98, 24, 25, 29.97, 30, 47.95, 48, 50, 59.94, 60, 72, 96, 100 and 120 frames per second with the support for drop-frame compensation with 29.97 and 59.94 rates.

* **Note about 30fps and 60fps drop-frame** : Even if those are listed as options in many software / hardware, those are useless since timecode is already in sync with video and real-time clock. Thus, there is no deviation to compensate. Other times the terms are misused as names for 29.87 and 59.94 drop-frame. In any case, 30DF and 60DF are not implemented as part of the library.
* **Note about 23.98fps and 47.95fps** : There is no standard way to compensate the deviation of those formats, so there is none implemented in this library. Those are then not in sync with real-time clock, accumulating a deviation of approximately 86 frames (3.6
seconds) in one hour of elapsed time.


## tcCoca Tool

**tcCoca** is a timecode converter / calculator command line tool using LibTC.

To build it just run `make`

```
tcCoca - timecode converter & calculator

Usage :
    tcCoca -F <format> <tc_value> [options]

    tc value can be either hh:mm:ss:ff timecode, frame number or any value
    associated with an edit rate.

Options :
        --help                        show this help
    -l, --list                        list all supported TC formats

    -F, --format            <format>  set the TC value format to <format>
    -R, --rate              <rate>    specify the edit rate of the input TC
                                      value - default is frame rate

    -c, --convert-to        <format>  convert TC value to the given <format>
        --convert-frames-to <format>  convert TC frame number to the given <format>
    -a, --add               <value>   add <value> to input TC value
    -s, --sub               <value>   subtract <value> from input TC value

Output :
    -h, --hmsf                        output TC as a time value hh:mm:ss:ff only
    -f, --frames                      output TC as a frame number only
    -n, --no-rollover                 don't rollover if TC is bigger than day limit


Examples :
    tcCoca -F 29.97DF 01:02:03:04 -a 02:10:01:07
    tcCoca -F 29.97DF 4147194251 -R 48000/1
    tcCoca -F 29.97DF 2589407
    tcCoca -F 29.97DF 01:00:00:00 -c 60
```

## Library usage

### First, set a new timecode

```c
struct timecode tc;

tc_set_by_hmsf( &tc, 23, 59, 59, 29, TC_29_97_DF );
// or
tc_set_by_string( &tc, "23:59:59:29", TC_29_97_DF );
// or
tc_set_by_frames( &tc, 2589407, TC_29_97_DF );

```

Sometimes you want to retrieve timecode from some value other than a frame number. One example is the *sample since midnight* from BWF/WAVE that holds a timecode value with sample accuracy. To do so, you can use `tc_set_by_unitValue()` by passing a value and its edit rate as a rational number :

```c
uint64_t value = 4147194251;
rational_t edit_rate = {48000, 1}; // value has 48000 units per second

tc_set_by_unitValue( tc, value, &edit_rate, TC_29_97_DF );
```

This function can be useful in the situations where you don't know if the value you're handling is a frame number, a sample number or anything else like in AAF files. `tc_set_by_unitValue()` will work in all those cases, even if the value is a frame number, as long as you pass the correct edit rate.

---

If you must use an unpredictable timecode format, you can call `tc_fps2format()` which returns the corresponding TC_FORMAT constant to be used with LibTC.

```c
float fps  = 29.97;
int isDrop = 1;

enum TC_FORMAT format = tc_fps2format( fps, isDrop );

if ( format == TC_FORMAT_UNK )
{
    printf( "Unsupported timecode format.\n" );
}
else
{
    tc_set_by_string( &tc, "23:59:59:29", format );
}
```

### Timecode reading

Setting a timecode using one of the above functions fills the entire `timecode` structure. The structure stores the TC in various forms :

```c
printf("%s\n", tc.string );
// prints 23:59:59;29

printf("%02u:%02u:%02u;%02u\n",
    tc.hours,
    tc.minutes,
    tc.seconds,
    tc.frames
);
// prints 23:59:59;29

printf("%u\n", tc.frameNumber );
// prints 2589407
```

### Timecode conversion

Converting a timecode means changing its format to another. There are two conversion methods.

The first one maintains the frame number and sets the timecode HMSF in accordance with the new format.

```c
struct timecode tc;


tc_set_by_string( &tc, "01:00:00:00", TC_29_97_DF );

//  29.97 FPS - drop frame
//  Timecode     : 01:00:00;00
//  Frame Number : 108000


tc_convert( &tc, TC_29_97_NDF );

//  29.97 FPS - non-drop frame
//  Timecode     : 01:00:03:18
//  Frame Number : 108000
```

The second method modifies the frame number so the timecode HMSF is maintained. If the destination format has a lower FPS than the input timecode frames, then the output frames are truncated to the new FPS - 1. For example, 00:00:00:59@60FPS converted to 25FPS will give 00:00:00:24.

```c
struct timecode tc;


tc_set_by_string( &tc, "01:00:00:00", TC_29_97_DF );

//  29.97 FPS - drop frame
//  Timecode     : 01:00:00;00
//  Frame Number : 108000


tc_convert_frames( &tc, TC_29_97_NDF );

//  29.97 FPS - non-drop frame
//  Timecode     : 01:00:00:00
//  Frame Number : 107892
```

### Timecode calculation

It is possible to add or subtract to timecodes. For that, both timecode operands must share the same format.
The result of the operation is stored in the first timecode structure.

```c
struct timecode tc_a;
struct timecode tc_b;

tc_set_by_string( &tc_a, "00:00:01:10", TC_29_97_DF );
tc_set_by_string( &tc_b, "01:00:02:05", TC_29_97_DF );

tc_add( &tc_a, &tc_b );

// tc_a : 01:00:03:15

tc_sub( &tc_a, &tc_b );

// tc_a : 00:00:01:10
```

## License

Copyright © 2018 Adrien Gesta-Fline<br />
tcCoca and LibTC are released under the __GNU AGPLv3__ : http://www.gnu.org/licenses/agpl-3.0.txt
