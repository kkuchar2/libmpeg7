#!/bin/bash

COMPOSE_FILE=docker-compose-dev.yml

docker-compose --file "$COMPOSE_FILE" build --force-rm --no-cache 

docker-compose --file "$COMPOSE_FILE" up
