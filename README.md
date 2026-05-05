# AR Campus ID Profile Card

This project extends the Unreal Handheld AR template into an AR campus ID profile card. On an iOS AR device, tap a detected surface or the printed campus card area to place a procedural holographic card. The card shows profile, skills, projects, and contact pages, supports touch manipulation, and plays a generated audio cue.

## Run

1. Open `ARProject.uproject` in Unreal Engine.
2. Build for iOS with ARKit support enabled.
3. Launch on an ARKit-capable iPhone or iPad.
4. Point the camera at a desk or the printed card from `Docs/CampusARCardMarker.svg`.
5. Tap once to place the AR campus card.

If AR plane detection is slow, the app falls back to placing the card in front of the camera so the demo can continue.

## Controls

- Tap once before placement: place the AR campus card.
- One-finger drag: rotate and slightly move the card.
- Two-finger pinch: scale the card.
- Tap `Profile`, `Skills`, `Projects`, or `Contact`: switch the information page.
- Tap `Audio`: play the generated intro sound effect.
- Tap `Reset`: restore the default page, rotation, and scale.

## Basic Requirements Mapping

- Creating a new object: `ACampusARCardActor` generates the campus card, hologram base, info panel, logo cube, and menu geometry in C++.
- Object manipulation: `ACampusARPlayerController` implements touch rotation, movement, and pinch scaling.
- Audio ambience: `ACampusARCardActor` generates and plays a short procedural sound when the Audio button is tapped.
- Pop-up menu: the 3D menu switches between Profile, Skills, Projects, Contact, Audio, and Reset.

## Advanced Application

The app turns a static campus ID card into an interactive AR profile. It can be used for student introductions, course demos, club recruitment, event networking, and portfolio presentation.
