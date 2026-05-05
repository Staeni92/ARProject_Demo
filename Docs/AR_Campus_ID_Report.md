# AR Campus Multi-Content Card Report

## Application Overview

The project is an AR multi-content campus card built on the Unreal Handheld AR template. A user opens the iOS AR app and points at a printed campus card marker. After the marker is recognized, the app places a tabletop AR display with four switchable content pages on that image target.

The application extends a traditional static campus card marker into an interactive campus information display for student introduction, course presentation, recruitment, and campus networking.

## Basic Requirement 1: Creating a New Object

The main AR object is created programmatically in C++ by `ACampusARCardActor`. It generates:

- A default student profile card with photo and basic information.
- A simulated CityUHK (Dongguan) homepage preview.
- A procedural 3D school emblem model.
- A reserved custom content page.
- 3D menu buttons and text.

This satisfies the new object requirement because the visible AR object is built by the project code rather than copied from the original template demo.

## Basic Requirement 2: Object Manipulation

The app supports mobile touch manipulation:

- One-finger dragging adjusts the AR display position relative to the recognized marker.
- Two-finger pinch changes the scale.
- Two-finger twist changes the angle and facing direction.
- The Reset button restores the original position, rotation, and scale.

This satisfies manipulation of position, size, and angle on an iOS AR device.

## Basic Requirement 3: Multi-Content Display

The AR card supports four content categories. The default page shows a personal profile, the website page previews the official English homepage, the emblem page shows a manipulable school emblem model, and the custom page is reserved for future personal content.

## Additional Feature: Pop-up Menu

The tabletop preview includes a 3D menu with these options:

- Profile
- Website
- Emblem
- Custom
- Reset

The menu changes the information shown in the AR display and supports the advanced application workflow.

## Demo Video Guide

Recommended recording sequence:

1. Launch the app and show the AR camera view.
2. Show the printed campus card marker.
3. Point the camera at the marker until the AR campus display appears.
4. Show the default Profile page with photo, name, student ID, and degree level.
5. Pinch to scale it and twist with two fingers to rotate it.
6. Tap Website to show the CityUHK (Dongguan) official homepage preview.
7. Tap Emblem to show the school emblem model, then scale and rotate it.
8. Tap Custom to show the reserved personal content page.
9. Tap Reset and explain that the application upgrades a static campus card marker into an interactive AR campus display.
