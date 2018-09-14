%echo off
FOR /F "tokens=* USEBACKQ" %%F IN (`git rev-parse --short HEAD`) DO (
SET var=%%F
)
ECHO "%var%" > githash.h