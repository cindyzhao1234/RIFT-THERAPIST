# Rift Therapist

Rift Therapist is a League of Legends performance analysis tool that will use the Riot Games API to analyse recent ranked games and generate simple, personalised insights.

The goal of this project is to practise C++ development, API integration, and player-focused software design through a League-related project.

## Project Status

Current stage: Phase 1 — C++ command-line MVP

So far, the project has:

- A basic C++ project structure
- CMake setup for building the program
- Git/GitHub version control setup
- A `.gitignore` file to avoid committing build files and environment secrets
- A `.env` file for storing the Riot API key locally

## Current Project Structure

```text
RIFT THERAPIST/
├── CMakeLists.txt
├── README.md
├── .gitignore
├── .env
└── src/
    └── main.cpp
```

## What I have learnt so far
- Never expose API key 

## API Key SetUp
This proejct requires a Riot Games API key. I created a local `.env` file: `RIOT_API_KEY=`

I created a `run.sh` script to make running the project easier. The script
- Loads Riot API key from .env file
- Builds c++ project using CMake
- Runs the compiled program