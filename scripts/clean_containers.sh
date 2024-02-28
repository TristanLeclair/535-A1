#!/bin/bash

source .env

# Function to print usage information
print_usage() {
    echo "Usage: $0 [--help]"
    echo "This script kills and removes all Docker containers based on the image '$IMAGE_NAME'."
    echo "No arguments are needed to run the script. Use '--help' to display this message."
}

# Check for --help argument
if [ "$1" == "--help" ]; then
    print_usage
    exit 0
fi

# Find all containers based on the specified image
containers=$(docker ps -a -q --filter ancestor=$IMAGE_NAME)

if [ -z "$containers" ]; then
    echo "No running containers found for image '$IMAGE_NAME'."
else
    # Kill and remove containers
    echo "Killing and removing containers for image '$IMAGE_NAME'..."
    docker rm $(docker kill $containers)
    echo "Containers based on the image '$IMAGE_NAME' have been killed and removed."
fi

