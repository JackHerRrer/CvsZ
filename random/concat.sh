#! /bin/bash

rm -f ./random_all.c
cat random.h        >> ./random_all.c
cat random_utils.h  >> ./random_all.c
cat random_utils.c  >> ./random_all.c
cat random.c        >> ./random_all.c

sed -i "s/#pragma once//" ./random_all.c
sed -i 's/#include "random.h"//' ./random_all.c
sed -i 's/#include "random_utils.h"//' ./random_all.c

echo "random_all.c succesfully generated"