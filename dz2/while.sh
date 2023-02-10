#!/bin/bash
echo "enter a b"
read a b
while [ $a -le $b ]
do
  echo "$a"
  a=$(( $a + 1 ))
done
