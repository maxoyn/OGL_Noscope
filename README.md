# OGL_Noscope

The goal here is to improve your “no scope ability”, which is considered an ability FPS games.<br/>
You have to hit an enemy with your sniper without scoping, which usually results in a one-shot-kill.<br/>

Lightning: There’s no lightning in the scene other than a point light aimed from the player. <br/>
Objects: Loaded using ASSIMP. There are two - an enemy and a weapon. <br/>
Aiming: The center of the screen has a crosshair consisted of 4 triangles. Dark and hardly visible as in those games there isn't one.<br/>
Game element: There is a timer to help measure your reaction and improve.  It's displayed when quitting. (Image 2)<br/>

How does it work?<br/>
To detect if the enemy is hit he has a bounding sphere. A ray cast is executed to see if you are aiming within it. <br/>
Then a pixel pick check to detect if you have accurately hit a part of the enemy. In our case this works, because<br/>
the background is a dark single color (night like) and we know we expect any other color as a result.<br/>
On every hit the enemy is positioned again at constant distance exactly behind you, <br/>
based on your current position and rotation and the timer is restarted.<br/>

<img src="https://github.com/maxoyn/OGL_Noscope/blob/master/images/myimg.jpg" width="700" height="550">
