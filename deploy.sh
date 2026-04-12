#!/bin/bash

#Deploy v1.5

FILE=run.exec

if [ "$1" = "--first-setup" ]; then
    mkdir croot
    mkdir hroot
    echo '#include <stdio.h>
    int main()
    {
        printf("Guts and Glory !\n");
        return 0;
    }' > base.c

    echo "default README.md workspace setup by deploy.sh" > README.md
fi

if [ -f "$FILE" ]; then
    rm run.exec
fi

logs=$(gcc base.c $(find croot -type f -name "*.c") -o run.exec 2>&1)

if [ -z "$logs" ]; then
    echo ":] good to go !"
    ./run.exec
else
    echo ":[ nope !"
    echo "$(echo "$logs")"
    exit
fi
