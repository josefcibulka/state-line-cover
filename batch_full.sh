./collifinder $1 tmp.kml
./set_coverer tmp.kml > unused_pairs_tmp
less unused_pairs_tmp | sort | uniq > unused_pairs
rm tmp.kml
rm unused_pairs_tmp
