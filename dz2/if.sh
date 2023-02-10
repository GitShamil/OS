#!/bin/bash
echo "enter a b"
read a b


if (( a > b )); then
  echo "first is bigger"
fi
if (( a < b )); then
  echo "second is bigger"
fi
