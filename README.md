# MLOG Chord Lab — Windows VST3 synthesizer

Original chord-stab synthesizer for rave, house, UK garage and hardgroove.

## Features
- Six chord voicings: Rave Minor, UKG Soul, Warehouse, Dream Pad, Hard Stab, Deep House
- One MIDI note plays a full chord
- Brightness, Drive, Release, Stereo Width and Output controls
- 16-note polyphony, MIDI note-on/off, stereo output, saved settings

## Get a Windows VST3
This ZIP contains **source code**, not a compiled Windows plugin. To build without installing a compiler locally:
1. Create a GitHub repository and upload all files **including the hidden `.github/workflows/build-windows.yml` file**.
2. Go to **Actions → Build Windows VST3 → Run workflow** (or push to main).
3. Download the `MLOG-Chord-Lab-Windows-VST3` artifact from the successful run.
4. Unzip and copy `MLOG Chord Lab.vst3` into `C:\Program Files\Common Files\VST3\`.
5. In FL Studio: **Options → Manage plugins → Find installed plugins**.

Build requires internet to download JUCE from GitHub during GitHub Actions. This is a source project and its Windows build has not been verified in this environment.

Sound-design note: this is an original synth for a broad genre, not a reproduction of any artist's copyrighted samples.
