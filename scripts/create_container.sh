#!/bin/bash

# Function to print the usage message
print_usage() {
    echo "Usage: $0 <container_name> [--help]"
    echo "Builds a Docker image from the current directory and manages containers based on this image."
    echo "Arguments:"
    echo "  <container_name>: Name(s) of the container(s) to manage."
    echo "Options:"
    echo "  --help: Display this help message and exit."
}

# Check for --help option
if [ "$#" -eq 0 ] || [ "$1" == "--help" ]; then
    print_usage
    exit 0
fi

# Source environment variables
source .env

ask_replace() {
  while true; do
    read -p "Container $1 already exists. Replace it? (y/n): " yn
    case $yn in
      [Yy]* )
        docker rm -f "$1"
        echo "Replacing container: $1"
        docker run -itd --name "$1" "$IMAGE_NAME"
        break;;
      [Nn]* )
        echo "Skipping container: $1"
        break;;
      * ) echo "Please answer yes (y) or no (n).";;
    esac
  done
}

docker build -t "$IMAGE_NAME" .

for CONTAINER_NAME in "$@"; do
  if docker ps -a --format '{{.Names}}' | grep -wq "$CONTAINER_NAME"; then
    ask_replace "$CONTAINER_NAME"
  else
    echo "Starting container: $CONTAINER_NAME with image $IMAGE_NAME"
    docker run -itd --name "$CONTAINER_NAME" "$IMAGE_NAME"
  fi
done

