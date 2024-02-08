#!/bin/bash

# Define the Docker image name
IMAGE_NAME="zcs"

# Container names
CONTAINER_NAME1="app"
CONTAINER_NAME2="service"

# Function to display help menu
show_help() {
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  -h, --help        Show this help message and exit."
    echo ""
    echo "This script builds an image named $IMAGE_NAME and creates two containers:"
    echo "- First container named '$CONTAINER_NAME1'"
    echo "- Second container named '$CONTAINER_NAME2'"
    echo "If containers already exist, you'll be prompted to remove them and create new ones."
    echo ""
    echo "Example:"
    echo "  $0"
}

# Function to check and handle existing containers
handle_existing_container() {
    local container_name=$1
    if [ "$(docker ps -aq -f name=^${container_name}$)" ]; then
        # Container exists
        echo "Container '$container_name' already exists."
        read -p "Do you want to remove it and create a new one? (y/N): " choice
        case "$choice" in 
            y|Y ) echo "Removing '$container_name'..."
                  docker rm -f $container_name
                  ;;
            * ) echo "Skipping '$container_name'."
                return 1
                ;;
        esac
    fi
    return 0
}

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -h|--help) show_help; exit 0 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
    shift
done

# Build the Docker image from the Dockerfile in the current directory
echo "Building Docker image $IMAGE_NAME..."
docker build -t $IMAGE_NAME .

# Handle existing containers and create new ones
if handle_existing_container $CONTAINER_NAME1; then
    docker run -itd --name $CONTAINER_NAME1 $IMAGE_NAME
    echo "Container '$CONTAINER_NAME1' has been created and started."
fi

if handle_existing_container $CONTAINER_NAME2; then
    docker run -itd --name $CONTAINER_NAME2 $IMAGE_NAME
    echo "Container '$CONTAINER_NAME2' has been created and started."
fi

