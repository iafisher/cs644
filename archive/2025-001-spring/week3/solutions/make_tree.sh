#!/bin/bash

d="/tmp/rm-rf-test"
rm -rf "$d"
mkdir -p "$d"/p1/p2/p3
touch "$d"/p1/p2/p3/f3
touch "$d"/p1/p2/f2
touch "$d"/p1/f1
echo "$d"
