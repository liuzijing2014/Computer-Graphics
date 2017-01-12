Subject 	: CSCI420 - Computer Graphics 
Assignment 2: Simulating a Roller Coaster
Author		: < Zijing Liu >
USC ID 		: < 5350978126 >

Description: In this assignment, we use Catmull-Rom splines along with OpenGL texture mapping to create a roller coaster simulation.

Core Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Uses OpenGL core profile, version 3.2 or higher - 
  Yes
2. Completed all Levels:
  Level 1 : - Yes
  level 2 : - Yes
  Level 3 : - Yes
  Level 4 : - Yes
  Level 5 : - Yes

3. Used Catmull-Rom Splines to render the Track - 
  Yes
4. Rendered a Rail Cross Section -
  Yes
5. Rendered the camera at a reasonable speed in a continuous path/orientation -
  Yes
6. Run at interactive frame rate (>15fps at 1280 x 720) -
  Yes
7. Understandably written, well commented code -
  Yes
8. Attached an Animation folder containing not more than 1000 screenshots -
  Yes. There are two videos for showing the animation!
9. Attached this ReadMe File -
  Yes
Extra Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Render a T-shaped rail cross section -
  No
2. Render a Double Rail -
  Yes. And it is texture mapped.
3. Made the track circular and closed it with C1 continuity -
  No
4. Added OpenGl lighting -
  No
5. Any Additional Scene Elements? (list them here)
  Yes. Texture-mapped wooden crossbars.
6. Generate track from several sequences of splines -
  No
7. Draw splines using recursive subdivision -
  Yes, with the minimum distance is 0.05f.
8. Modify velocity with which the camera moves -
  Yes, with the general speed formula:
  // af is the final acceleration and ai is the initial acceleration
  af = ai + dot product(gravity acceleration, tangent acceleration); 

  // vf is the final velocity and vi is the initial velocity
  vf = vi + at; 

  // d is the movement distance
  d = (vf + vi) * t / 2;

9. Create tracks that mimic a real world coaster - 
  No
10. Render environment in a better manner - 
  Yes. I used OpenGL cubemap(and cubemap texturing) to create the skybox, which 
  gives a better performance than using sky cube or sky dome.

Additional Features: (Please document any additional features you may have implemented other than the ones described above)
1. You can restart the roller coaster at any time by pressing keyword "R".
2. You can pause the ride by pressing keyword "S" in case you need some rest. Then resume the ride by pressing "S" again.
3. Determine coaster normals in a better manner than described in homework description.

Open-Ended Problems: (Please document approaches to any open-ended problems that you have tackled)
1.
2.

Keyboard/Mouse controls: (Please document Keyboard/Mouse controls if any)
1. Keyword "R" for replaying the roller coaster.
2. Keyword "S" for pause the ride if the ride is not paused.
3. Keyword "S" for resume the ride if the ride is paused.
3. Highly suggest not trying to rotate/translate/scale during your ride. I haven't updated the camera 
   rotation/translation/scale and those operations are from hw1, which is not meant for operating camera
   perspective.

Names of the .cpp files you made changes to:
1. hw2.cpp
2. basic.fragmentShader.glsl
3. basic.vertexShader.glsl
4. Makefile

Comments : (If any)
1. "make" to build the hw2, and then "./hw2 track.txt" to run the program.
2. I rendered the ground within the skybox. If you want to see the ground that rendered in the normal way, just search
   the following code inside hw2.cpp, uncomment them, and make. Then run "./hw2 track.txt", you will see a ground rendered
   in the normal way.

    /* Render Ground Begin */
    // glBindTexture(GL_TEXTURE_2D, gdTextureHandler);
    // glBindVertexArray(VaoGd);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)0);
    // glBindVertexArray(0);
    /* Render Ground End */


