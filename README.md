# Zero configuration Network

A library to run zero configuration services with multicasting across a network.

## Setup

We've created a script to automatically build and start small alpine linux containers to run apps and services in.

- `./scripts/create_containers.sh` to build and start the containers (By default will create 2 containers, one named service and one named app); both will have all the source code already built.
- `docker exec -it <name-of-container> sh` to open container shell prompt
- `./bin/apps/app` to start an app (choose executable in `./bin/`)

To create individual containers:
- `./scripts/create_container.sh <name-of-container>` will build and start a container with that name
- same steps as earlier
