NUM_ROOT_CONSES=010
NUM_BULK_CONSES=010
EDGE_FRACTION=0.0

SUFFIX=R${NUM_ROOT_CONSES}_B${NUM_BULK_CONSES}_F${EDGE_FRACTION}
BIN=../build-release/sim_cons_factory

time $BIN                                           \
     --num-root-conses $NUM_ROOT_CONSES             \
     --num-bulk-conses $NUM_BULK_CONSES             \
     --edge-fraction $EDGE_FRACTION                 \
     --num-steps 1000                               \
     --num-runs 100                                 \
     -q 0 -q 10 -q 20 -q 30 -q 40 -q 50 -q 60       \
     -q 70 -q 80 -q 90 -q 99                        \
     --garbage-steps 1                              \
     --recycle-steps 1                              \
     --num-bulk-conses-steps 5                      \
     --num-edge-rewire-steps 10                     \
     --quantile-output      quantile_${SUFFIX}.csv  \
     --output               runs_${SUFFIX}.csv      \
     --cons-fraction-output cfraction_${SUFFIX}.csv

