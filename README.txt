Subject 	: CSCI420 - Computer Graphics 
Assignment 2: Simulating a Roller Coaster
Author		: Aditya Aggarwal
USC ID 		: 4483470008

Description: In this assignment, we use Catmull-Rom splines along with OpenGL texture mapping to create a roller coaster simulation.

Core Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Uses OpenGL core profile, version 3.2 or higher - Y

2. Completed all Levels:
  Level 1 : - Y
  level 2 : - Y
  Level 3 : - Y
  Level 4 : - Y
  Level 5 : - Y

3. Used Catmull-Rom Splines to render the Track - Y

4. Rendered a Rail Cross Section - Y

5. Rendered the camera at a reasonable speed in a continuous path/orientation - Y

6. Run at interactive frame rate (>15fps at 1280 x 720) - Y

7. Understandably written, well commented code - Y (a few redundant parts here and there (move on spline and generatePoints From Spline) that could have been encapsulated into one function

8. Attached an Animation folder containing not more than 1000 screenshots - Y

9. Attached this ReadMe File - Y

Extra Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Render a T-shaped rail cross section - N

2. Render a Double Rail - N

3. Made the track circular and closed it with C1 continuity - N

4. Added OpenGl lighting - Y (Phong)

5. Any Additional Scene Elements? (list them here) SkyDome!!!!!!!

6. Generate track from several sequences of splines - (half Y) (multiple splines in one scene work - but did not create such a track)

7. Draw splines using recursive subdivision - N
 
8. Modify velocity with which the camera moves - Y

9. Create tracks that mimic a real world coaster - (half Y) (texture mapped tracks)

10. Render environment in a better manner - (other than SkyDome and Lighting, not really)

Additional Features: (Please document any additional features you may have implemented other than the ones described above)
1. 
2.

Open-Ended Problems: (Please document approaches to any open-ended problems that you have tackled)
1. For texture mapping the tracks I drew out the surface in 2D to make sure it looks natural - the entire surface perfectly wraps around the track - try mapping my vertices on to paper to see how :) 
2. Rendered a sky dome with my own code using a series of triangle strips. I rotated a start point at the top of the y axis like you would do with spherical coordinates in order to map each vertex. 
And did the texture mapping as an image that gets pinched as it reaches the top center of the hemisphere 
3. For Phong shading I removed the L component because I assumed all of them to be 1


Keyboard/Mouse controls: (Please document Keyboard/Mouse controls if any)
1. Rotation - mouse click
2. Shift - Scale
3. Translation - ctrl 
Ñ from last assignment

Names of the .cpp files you made changes to:
1. mainly hw2.cpp and the shaders

Comments : (If any)