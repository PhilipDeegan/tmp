set -exu

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" && cd $CWD

time (

  mkn build run -M soa_thrust.cpp -x hip -w mkn.gpu -a "-DMKN_GPU_ROCM" -O 2

) 1> >(tee $CWD/.mkn.sh.out ) 2> >(tee $CWD/.mkn.sh.err >&2 )
