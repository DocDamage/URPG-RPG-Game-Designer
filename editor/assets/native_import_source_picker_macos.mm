#import <AppKit/AppKit.h>
#import <dispatch/dispatch.h>

#include <cstring>

extern "C" bool urpgChooseMacOSImportSource(bool folder, char* output, size_t outputSize) {
    if (output == nullptr || outputSize == 0) {
        return false;
    }

    __block bool selected = false;
    auto runPanel = ^{
      @autoreleasepool {
          NSOpenPanel* panel = [NSOpenPanel openPanel];
          panel.canChooseFiles = folder ? NO : YES;
          panel.canChooseDirectories = folder ? YES : NO;
          panel.allowsMultipleSelection = NO;
          panel.canCreateDirectories = NO;
          panel.title = folder ? @"Choose Asset Source Folder" : @"Choose Asset Source File or Archive";

          if ([panel runModal] != NSModalResponseOK || panel.URL == nil) {
              return;
          }
          const char* path = [panel.URL.path fileSystemRepresentation];
          if (path == nullptr) {
              return;
          }
          const size_t length = std::strlen(path);
          if (length + 1 > outputSize) {
              return;
          }
          std::memcpy(output, path, length + 1);
          selected = true;
      }
    };

    if ([NSThread isMainThread]) {
        runPanel();
    } else {
        dispatch_sync(dispatch_get_main_queue(), runPanel);
    }
    return selected;
}
