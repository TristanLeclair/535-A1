# Zero configuration Network

A library to run zero configuration services with multicasting across a network.

## Setup

### Quickstart

We've created a script to automatically build and start small alpine linux containers to run apps and services in.

- `./scripts/create_container.sh app service` to build and start the containers
- `docker exec -it app sh` to open container shell prompt
- `./bin/apps/app` to start an app (choose executable in `./bin/`)
- `docker exec -it service sh` to open container shell prompt
- `./bin/services/service` to start an app (choose executable in `./bin/`)

#### RELAY

To connects nodes in LAN A to nodes in LAN B, you need a relay.

- `./scripts/clean_relay.sh` will create a container for the relay
- `docker exec -it relay sh` to open container shell prompt
- `./bin/relay/relay` to start the relay

### Manual

To create individual containers:
- `./scripts/create_container.sh <name-of-container>` will build and start a container with that name
- same steps as earlier

Or

- `./scripts/create_container.sh -i` will prompt you for the number of services and apps you want to create

### Cleanup

- `make clean` will clean out builds and libs locally and kill and remove all containers built with the "zcs" image (Configurable in `./.env`)

## Configuration

### .env

The .env file contains the name of the image to build with the local Dockerfile
