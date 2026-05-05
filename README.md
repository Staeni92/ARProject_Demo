# AR Campus Multi-Content Card

This project extends the Unreal Handheld AR template into a multi-content AR campus card. On an iOS AR device, point the camera at the printed campus card marker to recognize it and spawn a procedural tabletop display with four switchable views: student profile, CityUHK (Dongguan) website preview, school emblem model, and a reserved custom page.

## Run

1. Open `ARProject.uproject` in Unreal Engine.
2. Build for iOS with ARKit support enabled.
3. Launch on an ARKit-capable iPhone or iPad.
4. Point the camera at the printed card from `Docs/CampusARCardMarker.svg`.
5. Keep the marker in view until the AR campus display appears above it.

## Controls

- Scan the printed marker: place the AR campus display on the recognized image target.
- One-finger drag: move the display relative to the recognized campus card marker.
- Two-finger pinch: scale the display.
- Two-finger twist: rotate the display angle and facing direction.
- Tap `Profile`, `Website`, `Emblem`, or `Custom`: switch the displayed content.
- Tap `Reset`: restore the default page, rotation, and scale.

## Basic Requirements Mapping

- Creating a new object: `ACampusARCardActor` generates the tabletop AR display, profile card, website preview, school emblem model, custom placeholder, and menu geometry in C++.
- Object manipulation: `ACampusARPlayerController` keeps the spawned card anchored to the recognized image target while applying touch movement, twist rotation, and pinch scaling as relative transforms.
- Profile display: the default view shows the profile photo, name, student ID, and degree level.
- Website preview: the website view shows a static AR preview of `https://www.cityu-dg.edu.cn/en/home.html`.
- Emblem model: the emblem view shows a procedural 3D school emblem that can be scaled and rotated through the existing gestures.
- Pop-up menu: the 3D menu switches between Profile, Website, Emblem, Custom, and Reset.

## Advanced Application

The app turns a static campus ID card marker into an interactive AR campus display. It can be used for student introductions, course demos, recruitment events, and portfolio presentation.
