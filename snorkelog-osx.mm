/*

    To compile:

    clang++ -framework ApplicationServices -framework Foundation snorkelog-osx.mm -o snorkelog

    If you don't want Console to fill up... 

    clang++ -D SNRELEASE -framework ApplicationServices -framework Foundation snorkelog-osx.mm -o snorkelog

*/

#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#import <Foundation/Foundation.h>
#import <ApplicationServices/ApplicationServices.h>

using namespace std;
#define CAPTURE_THRESHOLD_SECS 60*20

#ifdef SNRELEASE
// Prevents bunch of output in Console
#define NSLog(...)
#endif

void listWin()
{
    NSArray*  windows =
        (NSArray*)CGWindowListCopyWindowInfo(
            kCGWindowListOptionOnScreenOnly, kCGNullWindowID);

    NSUInteger count = [windows count];
    for (NSUInteger i = 0; i < count; i++)
    {
        NSDictionary*   nswindowsdescription = [windows objectAtIndex:i];

        NSNumber* windowid =
            (NSNumber*)[nswindowsdescription objectForKey:@"kCGWindowNumber"];

        NSString* oname =
            (NSString*)[nswindowsdescription objectForKey:@"kCGWindowOwnerName"];

        NSNumber* onscreen =
            (NSNumber*)[nswindowsdescription objectForKey:@"kCGWindowIsOnscreen"];

        NSNumber* winlayer =
            (NSNumber*)[nswindowsdescription objectForKey:@"kCGWindowLayer"];

        //if(windowid && [winlayer intValue] == 0 )
        if(windowid)
        {
            NSLog(@"windowid %lu", [windowid longValue]);
            NSLog(@"oncreen? %d", [onscreen intValue]);
            NSLog(@"winlayer? %lu", [winlayer longValue]);
            NSLog(@"%@", oname);
            NSLog(@"---------");
        } // if

    } //for 
    [windows release];
} // listWin()

unsigned long firstVisibleWinId()
{
    NSArray*  windows =
        (NSArray*)CGWindowListCopyWindowInfo(
            kCGWindowListOptionOnScreenOnly, kCGNullWindowID);

    NSUInteger count = [windows count];
    for (NSUInteger i = 0; i < count; i++)
    {
        NSDictionary*   nswindowsdescription = [windows objectAtIndex:i];

        NSNumber* windowid =
            (NSNumber*)[nswindowsdescription objectForKey:@"kCGWindowNumber"];

        NSString* oname =
            (NSString*)[nswindowsdescription objectForKey:@"kCGWindowOwnerName"];

        NSNumber* onscreen =
            (NSNumber*)[nswindowsdescription objectForKey:@"kCGWindowIsOnscreen"];
        NSNumber* winlayer =
            (NSNumber*)[nswindowsdescription objectForKey:@"kCGWindowLayer"];

        if(windowid && [winlayer intValue] == 0 )
        {
            NSLog(@"%@", oname);
            [windows release];
            return [windowid longValue];
        } // if

    } //for 

    return 0; // Assume no valid window ID will be zero... ?
}

void dumpWindows()
{
    NSArray* windows = (id)CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
    NSLog(@"%@", windows);
    [windows release];
    windows = nil;
}

void screenCapture(CFStringRef imgPath)
{
    CGImageRef screenShot = CGWindowListCreateImage(
    CGRectInfinite,
        kCGWindowListOptionAll,/*kCGWindowListOptionOnScreenOnly,*/
        kCGNullWindowID,
        kCGWindowListOptionAll
        /*kCGWindowImageBoundsIgnoreFraming*/);

    CFStringRef type = CFSTR("public.jpeg");
    CFURLRef urlRef = CFURLCreateWithFileSystemPath(
        kCFAllocatorDefault, imgPath, kCFURLPOSIXPathStyle, false );
    CGImageDestinationRef idst = CGImageDestinationCreateWithURL( urlRef, type, 1, NULL );
    CGImageDestinationAddImage( idst, screenShot, NULL );
    CGImageDestinationFinalize( idst );
}

const string curr_date_time() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

const string curr_date() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);

    return buf;
}

void mreplace(string& str, const string& oldStr, const string& newStr)
{
  size_t pos = 0;
  while((pos = str.find(oldStr, pos)) != string::npos)
  {
     str.replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}


void savescreen()
{
    
    // The directory is the start of the path
    string fn = curr_date();
    int i = mkdir(fn.c_str(), S_IRWXU|S_IRGRP|S_IXGRP);
    //cout << "i=" << i << endl;

    fn.append("/");

    string tm(curr_date_time());
    mreplace(tm, ":", "_");
    fn.append(tm.c_str());
    fn.append(".jpg");

#ifndef SNRELEASE
    cout << "[SAVE] image filename = " << fn << endl;
#endif


    CFStringRef cfr;
    cfr = CFStringCreateWithCString(
        kCFAllocatorDefault,
        fn.c_str(),
        kCFStringEncodingMacRoman);
    screenCapture(cfr);

} // savescreen()


// Globals
unsigned long lastwindow = 0;
time_t lastcapture;

int main( int argc, char** argv)
{
    lastcapture = time(0);

#ifndef SNRELEASE
    cout << "lastcapture = " << (long)lastcapture << endl;
    cout << "lastwindow = " << (long)lastwindow << endl;
#endif

    /*
    // OS specific functionality testing...
    screenCapture(CFSTR("/tmp/screen.jpg"));
    dumpWindows();
    listWin();
    */

    unsigned long cw;
    time_t now;
    while (true)
    {
        cw = firstVisibleWinId();
        now = time(0);

#ifndef SNRELEASE
        cout << "current window id = " << (long)cw << endl;
        cout << "current timestamp = " << (long)now << endl;
#endif

        if (lastwindow != cw)
        {
            lastwindow = cw;
            lastcapture = now;
            savescreen();
        }
        else
        {

#ifndef SNRELEASE
            cout << "(current timestamp - lastcapture) = " <<
                (now - lastcapture) << " (seconds)" << endl;
#endif

            if ((now - lastcapture) > CAPTURE_THRESHOLD_SECS)
            {
                lastcapture = now;
                savescreen();
            }

        }
        sleep(5);
    }

}
