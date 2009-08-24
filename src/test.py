import shutil, os, os.path
import commands
import time
import csv

TEST_DATA = "data"
TEST_RESULTS = "results"

if not os.path.exists(TEST_DATA):
    os.mkdir(TEST_DATA)

if not os.path.exists(TEST_RESULTS):
    os.mkdir(TEST_RESULTS)


llfile = """# @ shell = /usr/bin/ksh
# @ output = %(results)s.out
# @ error = %(results)s.log
# @ wall_clock_limit = 0:4:00
# @ class= par128_3
# @ node = %(np)d
# @ node_usage = not_shared
# @ job_type = MPICH
# @ notification = error
# @ resources = ConsumableCpus(1)
# @ tasks_per_node = 1
# @ environment = GOTO_NUM_THREADS=1; OMP_NUM_THREADS=1; VIADEV_CLUSTER_SIZE=AUTO; VIADEV_DEFAULT_RETRY_COUNT=15; VIADEV_DEFAULT_TIME_OUT=22; VIADEV_NUM_RDMA_BUFFER=4; VIADEV_ADAPTIVE_RDMA_LIMIT=2; VIADEV_SQ_SIZE_MAX=64; VIADEV_DEFAULT_MAX_SG_LIST=1; VIADEV_MAX_INLINE_SIZE=80; VIADEV_SRQ_SIZE=2048; VIADEV_VBUF_TOTAL_SIZE=2048; VIADEV_VBUF_POOL_SIZE=512; VIADEV_VBUF_SECONDARY_POOL_SIZE=128; VIADEV_ENABLE_AFFINITY=0; DISABLE_RDMA_ALLTOALL=1; DISABLE_RDMA_ALLGATHER=1; DISABLE_RDMA_BARRIER=1
# @ queue

/CHPC/usr/local/mvapich/bin/mpirun \\
-np $LOADL_TOTAL_TASKS \\
-hostfile $LOADL_HOSTFILE \\
%(prog)s %(filter)s %(image)s out.pnm
"""


def get_image(size):
    size = str(size)
    fname = os.path.join(TEST_DATA, size + 'x' + size + '.pnm')
    if os.path.exists(fname):
        return fname
    os.system("./gen image %s %s %s" % (fname, size, size))
    return fname

def get_filter(size):
    size = str(size)
    fname = os.path.join(TEST_DATA, size + 'x' + size + '.filter')
    if os.path.exists(fname):
        return fname
    os.system("./gen filter %s %s %s" % (fname, size, size))
    return fname


def test_mpi(prog, np, img_size, filter_size):
    img = get_image(img_size)
    fil = get_filter(filter_size)
    cmd = "mpirun -np %d %s %s %s out.pnm" % (np, prog, fil, img)

    print
    print cmd
    print

    t = time.time()
    os.system(cmd)
    return time.time() - t

def test_mp(prog, np, img_size, filter_size):
    img = get_image(img_size)
    fil = get_filter(filter_size)
    cmd = "OMP_NUM_THREADS=%d %s %s %s out.pnm" % (np, prog, fil, img)

    print
    print cmd
    print

    t = time.time()
    os.system(cmd)
    return time.time() - t


def test_suite(progname, img_size, filter_size, nps):
    test = test_mpi
    if progname is 'mp':
        test = test_mp
    prog = './filter_' + progname

    hostname = commands.getoutput('hostname')
    results_name = '%s_%s_%sx%s_%sx%s.csv' % (hostname, progname,
                                              img_size, img_size,
                                              filter_size, filter_size)
    results_path = os.path.join(TEST_RESULTS, results_name)
    results = csv.writer(open(results_path, "wb"))
    results.writerow(['Number of processors', 'Seconds taken'])

    for np in nps:
        t = test(prog, np, img_size, filter_size)
        results.writerow([np, t])


def _test_chpc(progname, img_size, filter_size, np):
    prog = "./filter_" + progname
    results_name = 'chpc_%s_%dx%d_%dx%d_%d' % (progname,
                                               img_size, img_size,
                                               filter_size, filter_size,
                                               np)
    results_path = os.path.join(TEST_RESULTS, results_name)

    params = {
        'results': results_path,
        'np': np,
        'prog': prog,
        'filter': get_filter(filter_size),
        'image': get_image(img_size)
    }

    ll = file('run.ll', 'w')
    ll.write(llfile % params)
    ll.close()

    os.system('llsubmit run.ll')

def test_chpc():
    for progname in 'mpi_basic', 'mpi_neigh', 'mpi_ghost':
        for img_size in 1000, 2000, 5000, 10000, 20000, 50000:
            for filter_size in 5, 25, 51:
                for np in 20, 40, 60, 80, 100, 120:
                    print progname, img_size, filter_size, np
                    _test_chpc(progname, img_size, filter_size, np)
