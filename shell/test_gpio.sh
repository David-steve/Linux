#!/bin/bash

declare -i value=$3
for((i=$1;i<$2;i++));
do
echo `./gpio_test $i out $value`;
echo "GPIO$i value:$value"
done
