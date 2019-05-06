#!/bin/sh

echo \"$(git rev-parse --short HEAD)\" > githash.h
