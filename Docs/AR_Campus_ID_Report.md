# AR Campus Multi-Content Card Report

**Project title:** AR Campus Multi-Content Card

**Student name:** Biaoyao Zhang (张飙垚)

**Student ID:** 72404867

**Course project:** VR/AR Project

**Github:**  https://github.com/Staeni92/ARProject_Demo/tree/main

---

## Group Member Responsibilities

| Member | Student ID | Programming tasks | Report writing | Group coordination |
| --- | --- | --- | --- | --- |
| Biaoyao Zhang (张飙垚) | 72404867 | Implemented and refined the Unreal Engine AR application, including image tracking, AR object generation, touch interaction, page switching, profile photo loading, website preview, school emblem display, encyclopedia page, and runtime connection between image recognition and the generated AR card. | Wrote and organized the project report, README, setup notes, feature description, screen capture evidence notes, development effort summary, and demo guide. | Planned the project scope, selected assets, organized implementation order, tested the project, checked required resources, coordinated final packaging, and prepared the final submission materials. |

---

## Application Description

This project is an AR campus information card built with Unreal Engine 5.7 and the Handheld AR template. The application is designed for an iOS device that supports ARKit. When the user points the camera at the printed campus card marker, the app recognizes the image target and places an interactive AR display above it.

The AR display works like a digital campus name card. It can show personal information, a campus introduction video, a school emblem model, and a short campus encyclopedia page. The purpose is to upgrade a static card into a richer AR object that can be used for student introduction, campus promotion, recruitment, or project demonstration.

Main files and resources used in the implementation:

- AR image marker: `Content/Raw/StudentIDMarker.jpg`
- Profile photo: `Content/Raw/ProfilePhoto.jpg`
- Website/introduction video: `Content/Movies/campus-preview.mp4`
- School emblem model: `Content/Emblem/Optimized/Hitem3d-1778001575323_optimized/StaticMeshes/Hitem3d_1778001575323_Optimized.uasset`
- Main AR object code: `Source/ARProject/CampusARCardActor.cpp`
- AR tracking and gesture code: `Source/ARProject/CampusARPlayerController.cpp`

---

## Windows Demo to macOS + iOS Debugging Adaptation Summary

The original demo was adapted from a desktop-oriented Unreal template workflow into a macOS development and iOS real-device debugging workflow. The project was retargeted for Unreal Engine 5.7 on iOS, with `AppleARKit` enabled, `IOS` set as the target platform, and the AR startup map and C++ game mode configured for the campus card demo.

For real-device deployment, the iOS signing setup was updated to use the stable Bundle Identifier `com.gavinbycheung.arproject`, automatic Xcode signing, and the available Apple development team. This fixed earlier provisioning issues where Xcode could not find a matching development profile for the app.

The packaging path was also changed to make the app self-contained on the device. Instead of relying on Zen Store / network data streaming from the Mac, the project now uses Pak / IoStore packaging and disables Zen Store for the launch build:

```ini
UsePakFile=True
bUseIoStore=True
bUseZenStore=False
PackFilesForLaunch=PakNoCompress
```

When Unreal Editor reused stale Launch Profile settings, a manual `BuildCookRun` UAT workflow was used to explicitly build, cook, stage, deploy, and run the iOS app on the connected device. The project was also converted into a minimal C++ target so the iOS build could override the allocator for ARKit startup stability:

```csharp
if (Target.Platform == UnrealTargetPlatform.IOS)
{
	bOverrideBuildEnvironment = true;
	StaticAllocator = StaticAllocatorType.Ansi;
}
```

After these changes, the project can be developed on macOS, signed through Xcode, packaged with local cooked content, installed on an iOS device, and launched into the AR map for real ARKit testing.

---

## Basic Requirement 1: Creating a New Object

**Objective:** Create a new 3D object for the AR scene instead of only using the original template object.

**What was done:** I modelled a 3D CityU school emblem and imported it into the Unreal project as an AR display object. The model is stored in the project under `/Game/Emblem` and is loaded by `ACampusARCardActor` on the Emblem page. To make it suitable for mobile AR, I also performed polygon reduction / mesh optimization before using it in the client application.

**Final effect:** After the image marker is recognized, the user can switch to the Emblem page and view the optimized 3D CityU emblem in AR. The emblem keeps the recognizable CityU shape and raised logo details, while using a lighter mesh so it can be displayed more smoothly on an iOS AR device.

The final object includes:

- A custom 3D CityU emblem model.
- Raised logo geometry instead of a flat image.
- Reduced mesh complexity for better real-time AR performance.
- Integration into the AR application as a selectable Emblem content page.

**Screen capture / evidence:**

<img src="/Users/gavincheung/Library/Application Support/typora-user-images/image-20260506075035363.png" alt="image-20260506075035363" style="zoom:50%;" />

The submitted screen capture for this item shows the grey 3D CityU emblem model with the raised `CityU` text and rounded base shape. For the final demo, this model should be shown both in the modelling/preview view and inside the AR Emblem page.

**Particular effort:** The main effort for this item was not only creating the shape, but also preparing it for client-side AR display. A high-detail 3D emblem can be too heavy for mobile AR, so I optimized the model by reducing unnecessary polygons while keeping the main outline and raised logo details. This improves rendering performance and helps the AR application remain responsive when the user scales or rotates the emblem.

---

## Basic Requirement 2: Object Manipulation

**Objective:** Allow the AR object to be manipulated by the user, including position, size, and angle.

**What was done:** I implemented touch gesture handling in `ACampusARPlayerController`. The application reads one-finger and two-finger input from the mobile screen and applies the result to the AR campus card.

The supported operations are:

- One-finger drag: moves the AR display relative to the detected marker.
- Two-finger pinch: scales the AR display.
- Two-finger twist: rotates the AR display.
- Reset button: restores the original position, rotation, scale, selected page, and content-specific transforms.

The Emblem and Wiki pages also have more specific manipulation:

- On the Emblem page, dragging rotates the emblem model.
- Pinching and twisting on the Emblem page scale and rotate the emblem model.
- On the Wiki page, pinching and twisting adjust the encyclopedia panel.

**Final effect:** The user can reposition and resize the AR campus display after it appears. The object is no longer static; it can be adjusted to fit the viewing angle and physical environment.

**Screen capture / evidence:**

The final demo video should include a sequence showing the same AR card before and after dragging, pinching, and rotating. The implementation code for this requirement is mainly in `Source/ARProject/CampusARPlayerController.cpp`.

**Particular effort:** The gesture implementation has to distinguish between global card manipulation and page-specific manipulation. For example, when the Emblem page is active, pinch and twist control the emblem model itself. Otherwise, the same gestures control the whole AR card. This makes the application more flexible than a simple whole-object transform.

---

## Basic Requirement 3: Multi-Content Display

**Objective:** Provide multiple content items in the AR application.

**What was done:** I added a 3D menu and four switchable content pages:

- `Profile`: student information and profile photo.
- `Intro`: campus website/introduction preview using a video widget.
- `Emblem`: a 3D school emblem model that can be rotated and scaled.
- `Wiki`: a campus encyclopedia page with short information cards.
- `Reset`: restores the display to the default state.

Although the basic requirement asks for three items, this project includes more than three content states. The main three demonstrated items are Profile, Intro, and Emblem. Wiki is an extra content page.

**Final effect:** The user can tap the AR menu buttons to switch the display without leaving the AR scene. This turns the marker into a compact interactive information board.

**Screen capture / evidence:**

For final submission, the recommended screen captures are:

- Profile page after image recognition.
- Intro page showing the campus preview video.
- Emblem page showing the school emblem model.
- Optional Wiki page showing the encyclopedia cards.

The current resources used for the marker and profile page are:

<img src="../Content/Raw/StudentIDMarker.jpg" alt="Campus marker" style="zoom:33%;" />

![image-20260506075133841](/Users/gavincheung/Library/Application Support/typora-user-images/image-20260506075133841.png)

**Particular effort:** The page switching is managed by grouping Unreal scene components per page. When a menu button is tapped, the actor updates visibility for the relevant component group. The website page also starts and stops its media playback depending on whether the page is active.

---

## Advanced Requirement: Application Description

The advanced application idea is an AR campus identity and information hub. Instead of using AR only to display a decorative 3D object, the project connects a real-world student/campus card marker with multiple useful digital layers.

The application contains:

- Image recognition based on `StudentIDMarker.jpg`.
- A generated AR student profile card.
- Runtime loading of a personal photo from `Content/Raw/ProfilePhoto.jpg`.
- A campus introduction page using `campus-preview.mp4`.
- A school emblem model loaded from the project assets.
- A compact campus Wiki page.
- A 3D button menu for switching pages.
- Touch manipulation for the whole card and selected page content.

This application can be used in a campus open day, student portfolio presentation, orientation activity, or recruitment booth. A visitor can scan a printed card and immediately see personal, academic, and school information in AR.

**New ideas added:** The project goes beyond a simple marker-to-model demo by treating the marker as a multi-page AR interface. A possible future improvement is to let different student cards load different profile data from a local JSON file or a remote database. Another idea is to add audio narration or clickable links so the AR card can become a lightweight campus guide.
