CS350 Assignment 5 - Collision Detection System
Niklas Bieck
login: niklas.bieck

Controls:
WASD - Horizontal movement
QE   - Vertical movement
G    - Show/Hide GUI
Left Click + Drag - Change camera view (arcball)
All other functionality is accessible through the GUI

Notes:
- The octtree is capped to 3 levels starting from the root, but will also stop subdividing
when a node contains only a single object.
- I use the brute force approach to SAT, because the method described by Eberly required finding
extreme points along an axis, which seemed like a more complicated task
- I do store the previously successful axis for each pair of objects
- Reversing the animation simply means flipping its velocity. The animation is time-based, so it might not 
play exactly the same when reaching the boundaries of the box. (I snap the object into the scene bounding box)
- Single Stepping is achieved by pretending we have a standard 60 FPS frame time of 0.016s
- 64 bit compilation is supported and performs without warnings (mostly just me playing around)
