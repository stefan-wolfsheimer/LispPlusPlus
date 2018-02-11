NUM_ROOT_CONSES=100
NUM_BULK_CONSES=1000
EDGE_FRACTION=0.5

SUFFIX=R${NUM_ROOT_CONSES}_B${NUM_BULK_CONSES}_F${EDGE_FRACTION}
BIN=../build-release/sim_cons_factory 

time $BIN                                         \
     --num-root-conses $NUM_ROOT_CONSES           \
     --num-bulk-conses $NUM_BULK_CONSES           \
     --edge-fraction $EDGE_FRACTION               \
    --num-steps 1000                              \
    --num-runs 20                                 \
    -q 0 -q 2  -q 4 -q 6 -q 8 -q 10 -q 12         \
    -q 14 -q 16 -q 18 -q 19                       \
    --garbage-steps 1                             \
    --recycle-steps 1                             \
    --num-bulk-conses-steps 20                    \
    --num-edge-rewire-steps 20                    \
    --quantile-output      quantile_${SUFFIX}.csv \
    --output               runs_${SUFFIX}.csv     \
    --cons-fraction-output cfraction_${SUFFIX}.csv

