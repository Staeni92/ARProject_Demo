# AR Campus ID Profile Card Report

## Application Overview

The project is an AR Campus ID Profile Card built on the Unreal Handheld AR template. A user opens the iOS AR app, points at a desk or printed campus card, and taps to place a holographic campus profile. The AR card shows personal profile information, skills, project highlights, and contact details.

The application extends a traditional static campus card into an interactive profile tool for student introduction, course presentation, club recruitment, and campus networking.

## Basic Requirement 1: Creating a New Object

The main AR object is created programmatically in C++ by `ACampusARCardActor`. It generates:

- A campus card base panel.
- A floating holographic information panel.
- A 3D logo cube.
- A hologram base.
- 3D menu buttons and text.

This satisfies the new object requirement because the visible AR object is built by the project code rather than copied from the original template demo.

## Basic Requirement 2: Object Manipulation

The app supports mobile touch manipulation:

- One-finger dragging rotates the AR card.
- One-finger dragging also slightly adjusts its position.
- Two-finger pinch changes the scale.
- The Reset button restores the original rotation and scale.

This satisfies manipulation of position, size, and angle on an iOS AR device.

## Basic Requirement 3: Audio Ambience

The Audio button triggers a generated sound effect. The sound is created in C++ with `USoundWaveProcedural`, so the project does not depend on external audio assets. This gives the AR card an audio response when the user interacts with the profile.

## Additional Feature: Pop-up Menu

The holographic card includes a 3D menu with these options:

- Profile
- Skills
- Projects
- Contact
- Audio
- Reset

The menu changes the information shown on the floating panel and supports the advanced application workflow.

## Demo Video Guide

Recommended recording sequence:

1. Launch the app and show the AR camera view.
2. Show the printed campus card or desk area.
3. Tap to place the holographic campus ID profile.
4. Drag to rotate and move the card.
5. Pinch to scale it.
6. Tap Profile, Skills, Projects, and Contact.
7. Tap Audio to play the sound.
8. Tap Reset.
9. Explain that the application upgrades a static campus card into an interactive AR profile.
