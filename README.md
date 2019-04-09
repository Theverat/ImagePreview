# ImagePreview
A lightweight, un-cluttered image preview application

## Design Goals
- Fast ways to display images and convert them to different formats
- Workflow orientation: as few menus, buttons etc. as possible (using shortcuts as commands)
- Good image comparison capabilities: when switching between zooomed-in areas of images, the zoom is not reset

## Todo
- [x] better antialiasing for scaled down images available in QGraphicsView? (For photos it works good, but not for uniform patterns), this helped me a lot: http://qt-project.org/forums/viewthread/13643
- [x] When zooming in (more than 100%) AA should get turned off so you can analyze individual pixels
- [x] Fullscreen mode
- [x] Support for PSD files
