# ABrainFuck
Advanced brainfuck virtual machine implementation.

The advanced brainfuck allows a new great feature: it can open files!

In unix-like system, it means you can manage stdin stdout and files read/write, which is great! you can do a lot of things, such as read the /dev/random to get random numbers, or /proc/driver/rtc to get the current time!

the operator are mostly the same, just an important difference with ',': it does not access the stdin, but the current open file (by default stdout, so it gives you nothing).

to open a file (/dev/stdin for instance), you can use the switch operator '~'. It will open the file with the address string starting at the current memory pointer position (it should be ended by \0).

Once open, you are in "file mode" the '.' operation copy from the file to the memory, and the ',' operation copy from the memory to the file. To switch back to "memory mode", reuse the '~' operator.

When you read/write a file in memory mode, the file pointer is automatically shifted by one.

When you read/write the memory in file mode, the memory pointer is automatically shifted by one.

If you are in file mode, the reading overwrite the memory were was the pointer, so the file address. If you switch back to memory, you can shift the memory pointer and still get data from the file with the operator ','. But to shift the file pointer by a custom value, you need to reopen it.

Have fun!

##Compilation
    g++ ABrainFuck.cc -o AbrainFuck
##Usage
        ./ABrainFuck myscript.abf
##example:


    preparation of the stdout address:

    /
    >+++
    [>+++
    [<<+++++>>-]<-]
    <++.

    d
    >
    >+++++
    [>+++++
    [<<++++>>-]<-]
    <.

    e
    >
    >+++++
    [>+++++
    [<<++++>>-]<-]
    <+.

    v
    >
    >++++
    [>+++++
    [<<++++++>>-]<-]
    <--.

    /
    >
    >+++
    [>+++
    [<<+++++>>-]<-]
    <++.

    s
    >
    >+++++
    [>+++++
    [<<+++++>>-]<-]
    <----------.

    t
    [>+>+<<-]
    >>[-<<+>>]<+.

    d
    >
    >+++++
    [>+++++
    [<<++++>>-]<-]
    <.

    o
    >
    >++++
    [>+++++
    [<<++++++>>-]<-]
    <---------.

    u
    >
    >++++
    [>+++++
    [<<++++++>>-]<-]
    <---.

    t
    >
    >+++++
    [>+++++
    [<<+++++>>-]<-]
    <---------.

    \0
    >

    preparation of the stdin address
    /
    >
    >+++
    [>+++
    [<<+++++>>-]<-]
    <++.

    d
    >
    >+++++
    [>+++++
    [<<++++>>-]<-]
    <.

    e
    >
    >+++++
    [>+++++
    [<<++++>>-]<-]
    <+.

    v
    >
    >++++
    [>+++++
    [<<++++++>>-]<-]
    <--.

    /
    >
    >+++
    [>+++
    [<<+++++>>-]<-]
    <++.

    s
    >
    >+++++
    [>+++++
    [<<+++++>>-]<-]
    <----------.

    t
    [>+>+<<-]
    >>[-<<+>>]<+.

    d
    >
    >+++++
    [>+++++
    [<<++++>>-]<-]
    <.

    i
    >
    >++++
    [>+++++
    [<<+++++>>-]<-]
    <+++++.

    n
    >
    >++
    [>+++++
    [<<+++++++++++>>-]<-]
    <.

    \0
    >

    return to the begining
    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< it cannot be inferior to 0 in this VM
    go to stdin address
    >>>>>>>>>>>>
    open it, take 4 characters, in file mode it overwrite the address string, go back in memory mode
    ~....~
    <<<<<<<<<<<<<<<<<<<<< go to the begining, then open stdout, back to memory mode
    ~~ print the memory
    .>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>

    return to the begining
    <<<<<<<<<<<<<<<<<<<<<<<<<<<<< it cannot be inferior of 0 in this VM

    open stdin and take 4 characters, place them in memory, and go back in memory mode
    ~....~
    <<<<<<<<< go back to the begining
    ~~
    .>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>.>
