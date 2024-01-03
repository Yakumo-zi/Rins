#!/bin/bash
protoc --cpp_out=. ./*.proto
echo "build proto file successd!"