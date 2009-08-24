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
        t = test_mp(prog, np, img_size, filter_size)
        results.writerow([np, t])

