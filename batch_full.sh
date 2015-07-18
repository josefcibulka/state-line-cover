./collifinder $1 > tmp
./process_colli_output.sh tmp sets
./set_coverer < sets > unused_pairs_tmp
less unused_pairs_tmp | sort | uniq > unused_pairs
rm tmp
rm unused_pairs_tmp
