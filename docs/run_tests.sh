#!/bin/bash

if [ -d "build" ] 
then
     echo "Found build dir. Running tests..."
else
    echo "Directory /build does not exist. Running build chain..." 
    /bin/bash build.sh 
fi

cd build
make test

if [[ $? != 0 ]]; then
    cat Testing/Temporary/LastTest.log
fi
