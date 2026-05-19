#!/bin/bash

export $(cat .env | xargs)
cmake --build build
./build/rift_therapist