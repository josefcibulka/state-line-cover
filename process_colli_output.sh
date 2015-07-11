awk 'BEGIN {printing=0} {if(printing==1) {print} } /The sets/ {printing=1} ' $1 > /tmp/collinear_sets_tmp
sort < /tmp/collinear_sets_tmp > $2
rm /tmp/collinear_sets_tmp
