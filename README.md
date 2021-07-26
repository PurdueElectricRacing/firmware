# PER Component Firmware Projects
A mega-repository full of all firmware projects, build tools, and dependencies to create firmware modules for the car.

## Directory Structure
 ```
 cmake - CMake helper files for compiling common modules
 common - Common firmware modules shared across the codebase
 components - Firmware comonents for descrete MCUs
 ```

 ## Getting Started
 This project has a Dockerfile which will setup your build environment and setup the tools necessary to develop firmware. Once you install [Docker](https://docs.docker.com/get-docker/) you should be able to access command line tools by running `docker --help` for a list of available commands.

 Once Docker is installed, you can build the image defined by the `Dockerfile` by running:
 ```
 docker compose build develop
 ```
This command uses the `docker-compose.yaml` file to build the image with the tag of `my_per:latest`. Once you have built the image, running:
```
docker compose run develop
```
will place you into an interactive command line inside a container defined by the `my_per:latest` image. This is a minimal Ubuntu 20.04 distro and moves you to the `/per` directory. This directory is linked to the same directory in host machine which is defined in the `docker-compose.yaml` file. From there, you can run the:
```
build
``` 
command to build all of the components which will be placed in a newly created `output` folder.

To easily clean up build files, you can run
```
git clean -fdx
```
which will remove all untracked files from git. Be careful as any new files that have not been staged will also be removed.