Assignment #3: Ray tracing

FULL NAME: Zijing Liu


MANDATORY FEATURES
------------------

<Under "Status" please indicate whether it has been implemented and is
functioning correctly.  If not, please explain the current status.>

Feature:                                 Status: finish? (yes/no)
-------------------------------------    -------------------------
1) Ray tracing triangles                  yes and it is functioning correctly

2) Ray tracing sphere                     yes and it is functioning correctly

3) Triangle Phong Shading                 yes and it is functioning correctly

4) Sphere Phong Shading                   yes and it is functioning correctly

5) Shadows rays                           yes and it is functioning correctly

6) Still images                           yes and it is functioning correctly
   
7) Extra Credit (up to 20 points)
    a. Soft shadows. Check 003.jpg(table.scene) under image folder for the best illustration. 
       To achieve the best visual performance, I tweaked the bias that I used in my soft shadow implementation for each different scenes
       based on their light information. At the top of the hw3.cpp file, you could find a constant variable defined as SOFT_SHADOW_BIAS.
      
       #define SOFT_SHADOW_BIAS

       Here are detailed settings for each scene.
       test1.scene : 0.1
       test2.scene : 0.015
       spheres.scene : 0.5
       table.scene : 0.1
       SIGGRAPH.scene : 0.075

       Currently, it is set to 0.075. You don't need to change the value when you run with different scenes. 
       Soft shadow is functioning correctly with 0.075, but with a tweaked value, a better visual performance could be achieved :)
       
    b. Super sampling for antialiasing. Check 003.jpg(spheres.scene) and 004.jpg(table.scene) under image folder for the best illustration.

Notice: Because I use super sampling and soft shadowing, it is slow for rendering table.scene and SIGGRAPH.scene, 
especially SIGGRAPH.scene. I try my best to optimize the speed, but since all calculation is done on CPU side, I 
don't have much can do. Thanks for you patience, and my kindest suggestion would be to lay back and have a drink :)