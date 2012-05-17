SketchSynth: A drawable OSC control surface
===========================================

_SketchSynth_ lets anyone create their own control panels with just a
marker and a piece of paper. Once drawn, the controller sends
OpenSoundControl (OSC) messages to anything that can receive them; in
this case, a simple synthesizer running in Pure Data. It’s a fun toy
that also demonstrates the possibilities of adding digital interaction
to sketched or otherwise non-digital interfaces.

See [this video](https://vimeo.com/42053193) for more information.

My apologies for the name collision with Shape of Sound's iPad app,
_Sketch Synth 3D_.

Hardware Requirements
---------------------

To run the application, you need a webcam, a pico projector, and some
kind of arm or stand to mount them above a table. The software runs
fine on my four-year old ThinkPad, so it should work on any computer
made in past few years.

Software Requirements
---------------------

I have made no attempts to run this on anything other than Debian 6
(squeeze), but I don't expect problems running on other distributions.
There are no project files for XCode or CodeBlocks, but I don't these
will be hard to create if you need them.

You need the release version of openFrameworks 0.7 and Kyle McDonald's
[ofxCv addon](https://github.com/kylemcdonald/ofxCv).

If using the newest ofxCv, the project exists in the weird space between
openFrameworks 0.7 and the next version (0.71?). My system already had 
OpenCV 2.3.1 on it, so I used this library instead of the version
bundled with ofxOpenCv. If you use the development branches of
everything (oF and ofxCv), I think you'll be fine, but I haven't tested
it.

Projector Setup
---------------

The application assumes that the projector and desktop appear as one
large screen to the window manager, with the projector on the right of
the main screen. This required some hacking to my xorg.conf file to get
windows to maximize across both the projector and the laptop screen.

The size of this virtual screen should be set in `main.cpp` and you
should change the value of `screenSeparation` in `SketchSynth.h` to
the width of the primary display.

How to Use _SketchSynth_
------------------------

_SketchSynth_ starts in "setup" mode. The projector should show an
alignment rectangle. Click each corner of the rectangle, starting from
the upper left and moving clockwise. A shape resembling a plus sign
should appear in the center of the rectangle when alignment is complete.

Press `p` to enter "play" mode or `e` to enter "edit" mode. Press `s` to
return to the setup screen, if you need to update the alignment.

Nothing happens in "edit" mode, but it allows you to draw new controls or
move the paper around. Make sure nothing is excpet the paper is in the
frame when you switch to "play" mode: the first half-second or so is used
to set the background and detect the controls.

Once in "play" mode, just touch the controls. There are some tricks to
getting a good response, but they're pretty obvious after playing with
it for a few minutes.

OSC Format
----------

All OSC messages have the ID of the source control and a value.
Addresses are based on control type, for lack of a better scheme. IDs
start from 0 for each control type.

* `/paper/momentary <int> ["on"|"off"]`
* `/paper/toggle <int> ["on"|"off"]`
* `/paper/continuous <int> <float>`
* `/paper/count ["continuous"|"momentary"|"toggle"] <int>`
* `/paper/start`
* `/paper/stop`

Known Issues
------------

* Unwarping assumes that paper has dimensions proportional to standard
  letter paper (8.5" x 11"). Other size paper works, but controls are
  sometimes missed or detected incorrectly.
* There's no disticiton between a hand touching the paper and a hand
  over the paper. This can make things confusing.
* The paper, control, and hand detection is all very sensitive to
  illumination. If something doesn't work, try changing the lighting in
  the area, if you can.
