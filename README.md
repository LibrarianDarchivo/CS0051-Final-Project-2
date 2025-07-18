# CS0051-Final-Project-2

## INSTALL INSTRUCTIONS

### Oracle VirtualBox VM
1. Follow these YouTube videos on how to install Oracle VB VM.
   ∟ [Installing Oracle VirtualBox](https://youtu.be/homRENM8KVY?si=_LHDCBRonApBq6xM)
   ∟ [Configuring Ubuntu VM](https://youtu.be/xzEjychfD1Y?si=MvMcIHrPGddYxStm&t=589)

Note: Select **24.02.2 LTS** in the **Ubuntu Website**. It's what I used.

### Once inside

- Note: For me, copying and pasting code from outside of the VM __does not work__. To work around this, open this GitHub repository from within the VM itself then copy it into the .cpp files.
- Note: For me, selecting the entirety of the code and using `CTRL + K` to cut all at once __does not work__. Instead, you can select the first line of your code and **hold** `CTRL + K` to erase the existing contents to paste in the new updated code from GitHub. (If you find a work-around please tell us because I legit have no idea what I'm doing lmao)
- Note: You can press `CTRL + L` to "clear" the terminal. Makes it easier to read. (All it does is move the cursor down a few lines lmao)

1. Open terminal and type in the following:

  `mkdir pusoy_clash`

  `cd pusoy_clash`

   *`mkdir` creates a new directory called **pusoy_clash** and uses that directory using `cd`*

2. To create the two files: `server` and `client`

  `nano server.cpp`

  ∟ After you paste in the code, do `CTRL + O` then `ENTER` to save the file and `CTRL + X` to exit

  ∟ Do the same for the next file

  `nano client.cpp`

### To run (compile) files

1. Open terminal and do the following:

  **1. For server terminal:**
    `cd pusoy_clash`
    `g++ server.cpp -o server -pthread`

  **2. For client terminal**
    `cd pusoy_clash`
    `g++ client.cpp -o client -pthread`

- Note: You need only run the server.cpp once for the **1 server terminal**. For the client terminals, the program will not begin unless **4 players are connected**, meaning you need to have 4 seperate terminals running with client.cpp to start the program.

### Limitations

- There is currently no "retry" function, so you have to restart the entire process of recompiling the files again.
- Might be some bugs, idk
