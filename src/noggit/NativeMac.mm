// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifdef __APPLE__

#include <iostream>
#include <string>
#include <noggit/Log.h>
#import "Native.hpp"
#import <Cocoa/Cocoa.h>

const std::string kNotFoundTitle = "Noggit was unable to locate World of Warcraft.";
const std::string kNotFoundMessage = "Click OK to select the location of your Wrath of the Lich King (3.3.5) installation.";
const std::string kMismatchTitle = "WoW version mismatch";
const std::string kMismatchMessage = "The WoW binary found by Noggit wasn't the expected version (3.3.5). Noggit may crash or behave strangely.";

int Native::showAlertDialog(std::string title, std::string message)
{
    NSAlert *fileAlert = [[NSAlert alloc] init];
    fileAlert.messageText = [NSString stringWithCString:title.c_str() encoding:NSUTF8StringEncoding];
    fileAlert.informativeText = [NSString stringWithCString:message.c_str() encoding:NSUTF8StringEncoding];
    
    int result = [fileAlert runModal];
    [fileAlert release];
    
    return result;
}

BOOL checkWoWVersionAtPath(NSString *filePath)
{
    NSString *pathWithBundle = [filePath stringByAppendingPathComponent:@"World of Warcraft.app"];
    NSBundle *wowBundle = [NSBundle bundleWithURL:[NSURL fileURLWithPath:pathWithBundle]];
    NSDictionary *bundleInfo = [wowBundle infoDictionary];
    NSString *version = bundleInfo[@"CFBundleVersion"];
    
    return [version isEqualToString:@"3.3.5"];
}

std::string Native::getGamePath()
{
    Native::showAlertDialog(kNotFoundTitle, kNotFoundMessage);
    
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.canChooseDirectories = YES;
    openPanel.canChooseFiles = NO;
    
    if ([openPanel runModal] == NSFileHandlingPanelOKButton) {
        NSString *path = openPanel.URL.relativePath;
        
        if (!checkWoWVersionAtPath(path)) {
            Native::showAlertDialog(kMismatchTitle, kMismatchMessage);
            Log << "NoggitCocoa: WoW binary version mismatch" << std::endl;
        }
        
        return std::string([path UTF8String]);
    }
    
    return std::string("");
}

std::string Native::getArialPath()
{
    // Size does not affect ability to find font, but is required by method
    // This searches fonts at system-stock (/System/Libray/Fonts/), system user (/Library/Fonts/), and user (~/Library/Fonts/) levels
    NSFont *font = [NSFont fontWithName:@"Arial" size:12];
    
    if (!font) {
        // User doesn't have Arial for some reason
        // Let's try Helvetica since it's stock with all Macs since 1984 and has nearly identical metrics
        font = [NSFont fontWithName:@"Helvetica" size:12];
        Log << "NoggitCocoa: Substituting Helvetica for Arial." << std::endl;
    }
    
    if (!font) {
        // Subtitute didn't work, user has likely wrecked his OS install. Bailing.
        // We could instead subtitute the system sans-serif here if we wanted to.
        Log << "NoggitCocoa: Couldn't find Arial or Helvetica!" << std::endl;
        return "";
    }
    
    CTFontDescriptorRef fontRef = CTFontDescriptorCreateWithNameAndSize ((CFStringRef)[font fontName], [font pointSize]);
    CFURLRef url = (CFURLRef)CTFontDescriptorCopyAttribute(fontRef, kCTFontURLAttribute);
    NSString *fontPath = [NSString stringWithString:[(NSURL *)url path]];
    CFRelease(fontRef);
    CFRelease(url);
    
    return [fontPath UTF8String];
}

//
//std::string NativeMac::applicationSupportPath()
//{
//    NSArray *urls = [[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
//    
//    if (urls.count) {
//        return std::string([[[urls[0] relativePath] stringByAppendingString:@"/Noggit/"] UTF8String]);
//    }
//    
//    return "";
//}

#endif
