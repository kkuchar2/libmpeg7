#!/bin/bash

COMPOSE_FILE=docker-compose-dev.yml

docker-compose --file "$COMPOSE_FILE" up --force-recreate --build