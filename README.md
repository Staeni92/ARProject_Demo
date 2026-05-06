# AR Campus Multi-Content Card

This project extends the Unreal Engine Handheld AR template into an interactive campus AR card. It runs on ARKit-capable iPhone and iPad devices. After the app recognizes the campus card image marker, it spawns an interactive AR display above the marker with profile information, a campus website preview, a school emblem model, and wiki-style content pages.

## Project Environment

- Unreal Engine: `5.7`. The project file `ARProject.uproject` uses `EngineAssociation` set to `5.7`.
- Target platform: iOS. The project `TargetPlatforms` setting is configured for `IOS`.
- Development system: macOS is recommended because iOS packaging and device deployment require Xcode and Apple signing.
- Xcode: Install a version that supports the current iOS SDK, and open Xcode once before building so you can accept the license agreement.
- iOS device: An ARKit-capable iPhone or iPad running iOS 15 or later.
- Apple Developer account: Device installation requires a valid team, certificate, and provisioning profile. The project currently enables automatic signing and uses the bundle identifier `com.gavinbycheung.arproject`.

## Main Dependencies

The project uses the following Unreal modules and plugins:

- `AppleARKit`: iOS ARKit runtime support.
- `AugmentedReality`: AR sessions, image tracking, and AR geometry queries.
- `ProceduralMeshComponent`: Runtime generation for the AR display stand and UI geometry.
- `UMG`, `Slate`, `SlateCore`: Page and media UI.
- `MediaAssets`, `AudioMixer`: Campus website preview video playback.
- `ImageWrapper`, `RenderCore`: Runtime JPG loading from `Content/Raw` into transient textures.

## Assets

- Tracking marker: `Content/Raw/StudentIDMarker.jpg`
- Profile photo: `Content/Raw/ProfilePhoto.jpg`
- Website preview video: `Content/Movies/campus-preview.mp4`
- Emblem model: `Content/Emblem/Optimized/Hitem3d-1778001575323_optimized/StaticMeshes/Hitem3d_1778001575323_Optimized.uasset`
- Printable marker reference: `Docs/CampusARCardMarker.svg`

At runtime, the app creates an AR candidate image from `Content/Raw/StudentIDMarker.jpg`. The configured physical marker size is `8.6 cm x 5.4 cm`. For testing, print or display the marker at the same aspect ratio, and keep it flat, clear, and unobstructed.

## Setup

1. Install Unreal Engine 5.7.
2. Install Xcode, then confirm that the command line tools point to the active Xcode installation:

   ```sh
   xcode-select -p
   ```

3. Open `ARProject.uproject`. If Unreal asks to regenerate C++ project files or compile modules on first launch, confirm the prompt.
4. Confirm that these plugins are enabled:
   - `AppleARKit`
   - `ProceduralMeshComponent`
5. In Unreal Editor, open `Project Settings > Platforms > iOS` and check the signing configuration:
   - Bundle Identifier: `com.gavinbycheung.arproject`
   - Automatic Signing: Enabled
   - Team ID: Replace this with your own Apple Developer Team ID, or use a certificate and provisioning profile available on your Mac.
6. Confirm that `Project Settings > Maps & Modes` uses this default map:

   ```text
   /Game/HandheldARBP/Maps/HandheldARBlankMap
   ```

7. Confirm that `Project Settings > Project > Packaging` includes the runtime assets:
   - `Content/Raw` is packaged as a UFS directory.
   - `/Game/Emblem` is included in Always Cook.

## Running the Project

### Open in the Editor

1. Double-click `ARProject.uproject`, or open it from the Unreal Editor project browser.
2. Wait for C++ compilation and shader compilation to finish.
3. You can inspect the map, models, materials, and C++ logic in the editor. The full ARKit image recognition flow requires running on a physical iOS device.

### Deploy to iPhone or iPad

1. Connect an ARKit-capable iPhone or iPad over USB, then trust the computer on the device.
2. In the Unreal Editor toolbar, select the target iOS device.
3. Use `Platforms > iOS > Package Project` to package the app, or use `Launch` to deploy directly.
4. If signing fails, check the Team ID, certificate, provisioning profile, and bundle identifier under `Project Settings > Platforms > iOS`.
5. After installation, open the app on the device and allow camera access.
6. Point the camera at the printed or screen-displayed image that matches `Content/Raw/StudentIDMarker.jpg`.
7. Keep the marker visible in the camera view. After recognition succeeds, the AR display appears above the marker.

### Build the Editor Target from the Command Line

To verify that the C++ project builds on macOS, run:

```sh
"/Users/Shared/Epic Games/UE_5.7/Engine/Build/BatchFiles/Mac/Build.sh" ARProjectEditor Mac Development -Project="/Users/gavincheung/Documents/Unreal Projects/ARProject_Demo/ARProject.uproject" -WaitMutex
```

This command only builds the Mac editor target. It is not the same as packaging for an iOS device. For iOS deployment, use the Unreal Editor iOS platform menu so signing, cooking, and installation are handled correctly.

## Controls

- Scan marker: Recognize the campus card marker and spawn the AR display.
- One-finger drag: Move the display relative to the tracked image.
- Two-finger pinch: Scale the display.
- Two-finger rotate: Adjust the display orientation.
- One-finger drag on the `Emblem` page: Rotate the emblem model.
- Two-finger pinch or rotate on the `Emblem` page: Scale or rotate the emblem model.
- Tap `Profile`, `Intro`, `Emblem`, or `Wiki`: Switch display pages.
- Tap `Reset`: Restore the default page, rotation, and scale.

## Feature Overview

- Object creation: `ACampusARCardActor` generates the AR display stand, profile card, website preview, emblem model, and menu in C++.
- Image recognition: `ACampusARPlayerController` starts the AR session and registers `StudentIDMarker.jpg` as a runtime candidate image.
- Object manipulation: The display is anchored to the tracked image while still supporting touch movement, rotation, and scaling.
- Profile information: The profile page shows a photo, name, student ID, major, and academic level.
- Website preview: The `Intro` page plays the campus website preview video from `Content/Movies/campus-preview.mp4`.
- Emblem model: The emblem page loads the 3D model under `/Game/Emblem` and supports independent scaling and rotation.
- Wiki content: The `Wiki` page uses `UCampusEncyclopediaWidget` to show campus encyclopedia content, with independent scaling and rotation support.

## Use Cases

This project turns a static campus card marker into an interactive AR campus display. It can be used for student introductions, course demonstrations, admissions promotion, portfolio presentations, and campus event guidance.
