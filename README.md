# OGL_Noscope

The goal here is to improve your “no scope ability”, which is considered an ability FPS games.
You have to hit an enemy with your sniper without scoping, which usually results in a one-shot-kill.

Lightning: There’s no lightning in the scene other than a point light aimed from the player. /n
Objects: Loaded using ASSIMP. There are two - an enemy and a weapon. 
Aiming: The center of the screen has a crosshair consisted of 4 triangles.
Game element: There is a timer to help measure your reaction and improve.

How does it work?
To detect if the enemy is hit he has a bounding sphere. A ray cast is executed to see if you are aiming within it. 
Then a pixel pick check to detect if you have accurately hit a part of the enemy. In our case this works, because
the background is a dark single color (night like) and we know we expect any other color as a result.
On every hit the enemy is positioned again at constant distance exactly behind you, 
based on your current position and rotation and the timer is restarted.
