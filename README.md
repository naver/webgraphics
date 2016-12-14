# webgraphics
The purpose of WebGraphics is simple: Making WebKit render faster.
WebGraphics is based on Mozilla's nice graphics implementation(gfx/2d), which is being used for FireFox web browser.
The project also can be used for any other projects which need graphics backend, as it designed to be. Have fun!

## Prerequisite
* CMake - 3.4 preferred.
* Visual Studio 2015 - On Windows.
* WebKit, patched with WebGraphics - On Windows. Needed because WebGraphics link to WebKit's ANGLE library.

## Getting Started
### Windows
With CMake installed, do like this:
```
mkdir build
cd build
cmake -G "Visual Studio 14 2015" -D WEBKIT_LIBRARIES_DIR=<webkit-directory> -D ANGLE_LIBRARY_DIR=<ANGLE-directory> ..
```
Then open webgraphics.sln, Hit "Build Solution". or type `cmake --build .`

### Other Platforms
Not yet supported. For Android, WebGraphics should use Skia.

## Integrate to WebKit
### WebKit with Cairo
WebKit already ported to Cairo graphics. Cairo is good enough for render web pages to the screen but has issues on performance limitation, as noted by Mozilla's Firefox team. The performance downside is obvious when you build and run WebKit's WinCairo port on your Windows system: It cannot take any advantages from modern features, such like Direct2D, which Windows OS offer.

Then we could think of porting WebKit to Direct2D, for Windows. But it's not a trivial job one could do in a few days. And what about other platforms?
If we move to Android, we surely will want to use Skia as graphics backend for WebKit. Moreover, there would be other platforms which give us better performance when we stick to Cairo.

And even if we port WebKit to many graphics backend with no problem, there will be difficulties to maintain it.

### Cairo and WebGraphics
Mozilla had done a great job on Firefox's graphics system. Rather than reinvent the wheel, we decided to adopt their work for WebKit. WebGraphics is bridge between WebKit and Mozilla and the main idea is really simple - Cairo without Cairo. That is, WebGraphics uses Cairo's surfaces and types but does not use Cairo's rasterization through their APIs.

## Integrate to Other Projects
To use WebGraphics for your project, first you should include `cairo.h` and other Cairo headers you need. Note that you only need a few Cairo APIs which create surfaces and data types. Then include `WebGraphicsContext.h` and draw with WebGraphicsContext APIs to the surface you created with Cairo.

## Resources
* External Links

## Code Examples
WebGraphics includes a test application used for development and demonstration. Refer to `gfxtest` project.
