#!/bin/bash

declare -i value=$3     //将参数3转为整数
for((i=$1;i<$2;i++));   //需要有两对括号
do
./gpio_test $i out $value;
echo "GPIO$i value:$value"
done
