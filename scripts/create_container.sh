#!/bin/bash

# Function to print the usage message
print_usage() {
    echo "Usage: $0 [--interactive | -i] [--help]"
    echo "Builds a Docker image from the current directory and manages containers based on this image."
    echo "Options:"
    echo "  --interactive, -i: Enter interactive mode to specify the number of containers to create."
    echo "  --help: Display this help message and exit."
}

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

create_containers() {
    local type=$1
    local count=$2
    for ((i=1; i<=count; i++)); do
        local container_name="${type}${i}"
        echo "Creating container: $container_name with image $IMAGE_NAME"
        docker run -itd --name "$container_name" "$IMAGE_NAME"
    done
}

interactive_mode() {
    read -p "How many services do you want to create? " service_count
    read -p "How many apps do you want to create? " app_count
    create_containers "service" $service_count
    create_containers "app" $app_count
}

# Main script logic
if [ "$1" == "--help" ]; then
    print_usage
    exit 0
elif [ "$1" == "--interactive" ] || [ "$1" == "-i" ]; then
    docker build -t "$IMAGE_NAME" .
    interactive_mode
    exit 0
fi

if [ "$#" -eq 0 ]; then
    print_usage
    exit 1
fi

docker build -t "$IMAGE_NAME" .

for CONTAINER_NAME in "$@"; do
  if docker ps -a --format '{{.Names}}' | grep -wq "$CONTAINER_NAME"; then
    ask_replace "$CONTAINER_NAME"
  else
    echo "Creating container: $CONTAINER_NAME with image $IMAGE_NAME"
    docker run -itd --name "$CONTAINER_NAME" "$IMAGE_NAME"
  fi
done

