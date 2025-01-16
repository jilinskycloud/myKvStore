#!/bin/bash

# Folder name to create
FOLDER_NAME="/root/new_root_folder"

# Check if the script is run as root
if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit 1
fi

# Check if the folder already exists
if [ -d "$FOLDER_NAME" ]; then
  echo "Folder '$FOLDER_NAME' already exists."
else
  # Create the folder
  mkdir "$FOLDER_NAME"
  if [ $? -eq 0 ]; then
    echo "Folder '$FOLDER_NAME' created successfully."
  else
    echo "Failed to create folder '$FOLDER_NAME'."
  fi
fi
