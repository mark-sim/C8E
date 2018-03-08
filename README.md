# C8E
### CHIP-8 Emulator written in C++11


This was my first project to get an insight of *THE SCARY EMULATOR WORLD.*  


All the games in ROM works!  
Even though the OpenGL renderer is not as good as I want it to be (maybe I'll try making it better next time),  
All the games work and they are accurate.  


If you want to try out my CHIP-8 Emulator, make sure you have [OpenGL](https://opengl.en.softonic.com/) installed (if you're a mac user you probably have it already by default) and [C++ compiler](https://gcc.gnu.org/) installed.  



Then run   
```
g++ -std=c++11 *.cpp -framework OpenGL -framework GLUT -o chip8.exe
```
in the directory that contains .cpp files.  



After that, you can run any games you want by  
```
./chip8.exe ../rom/[NAME OF ROM]
```



for example,  
```
./chip8.exe ../rom/PONG
```



The keyboard layout is as follows:  
```
1 2 3 4
Q W E R
A S D F
Z X C V
```
To be honest, I don't know the keys for each game either.  
Play around with it, it won't be too hard to figure it out.  

This is the *famous space invaders*  
<img src="https://github.com/marksim5/C8E/blob/master/demo/demo.gif?raw=true" width="480" height="256"/>

This project's been very fun from start to beginning.  
Used some concepts thought in school such as instruction fetching & decoding and LOTS of research to learn the underlying hardware.  
Good thing it was well documented though ┌( ಠ‿ಠ)┘.  