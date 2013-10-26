snorkelog
=========

Spy on yourself!

This source provides a simple screengrab hack (I hesitate to say 'program') for
busy developers (or others) who need to report their daily activity. Throughout
the day, grabs screenshots as windows change and activity occurs, creating a
visual log of the days events.

## How it works (basically)

    init global last window id *L*
    init global lastsave time *T*

    loop every *n* seconds:
        get current foreground window *C*
        if *L* is not *C*:
            save screenshot as *timestamped image file*
        else:
            if *T* is greater than some threshold (every 20 minutes?):
                save screenshot as *timestamped image file*

This can be built on Windows and OSX.

## Windows

In the *Visual Studio Command Line* (I used 2012), do

    cl.exe /I src /D DEBUG /EHsc main.cpp /link user32.lib Gdi32.lib Gdiplus.lib /OUT:snorkelog.exe

## OSX

To build:

    clang++ -D SNRELEASE -framework ApplicationServices -framework Foundation snorkelog-osx.mm -o snorkelog

Note, for both platforms, see the source file comments for more details

## License

This source is public domain, use at your own risk. Patches and forks welcome. ;)
