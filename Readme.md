## Make The Image
From the directory where Dockerfile exists run:
```bash
docker build -t s0 . 
```
Notice that '.'? That specifies 'current' directory, and in general, should specify where your Dockerfile is. This command creates a docker image named 's0' based on the recipe in the Dockerfile.

## Compose Up
Then from the directory where 'docker-compose.yaml' exists, run:
```bash
docker compose up
```
This used to be a different command in v1, more info here: https://docs.docker.com/compose/compose-v2/
Another alternative way to get this running on Windows (I don't know about Mac) is from the Docker GUI (Docker Desktop). Linux has it too: https://www.docker.com/blog/the-magic-of-docker-desktop-is-now-available-on-linux/ I also recommend using the docker extension for vscode. 

## SSH into machines
Now you have 2 containers running. Attach a shell to either of them with the method of your choice. (vscode extension/ docker desktop GUI/ docker attach \<container-name\>)
