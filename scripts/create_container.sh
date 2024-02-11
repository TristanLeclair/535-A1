#!/bin/bash

ask_replace() {
  while true; do
    read -p "Container $1 already exists. Replace it with a fresh instance? (y/n): " yn
    case $yn in
      [Yy]* )
        docker rm -f "$1"
        echo "Replacing container: $1"
        docker run -d --name "$1" "$IMAGE_NAME"
        break;;
      [Nn]* )
        echo "Skipping container: $1"
        break;;
      * ) echo "Please answer yes (y) or no (n).";;
    esac
  done
}

if [ "$#" -eq 0 ]; then
  echo "Usage: $0 <container_name> ..."
  exit 1
fi

IMAGE_NAME="zcs"

docker build -t "$IMAGE_NAME" .

for CONTAINER_NAME in "$@"; do
  if docker ps -a --format '{{.Names}}' | grep -wq "$CONTAINER_NAME"; then
    ask_replace "$CONTAINER_NAME"
  else
    echo "Starting container: $CONTAINER_NAME with image $IMAGE_NAME"
    docker run -itd --name "$CONTAINER_NAME" $IMAGE_NAME
  fi
done
