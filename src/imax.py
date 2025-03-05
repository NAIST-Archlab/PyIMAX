from pyimax import IMAXArray, sysinit
import numpy as np
import datetime
import threading


def generate_2d_array(rows, cols):
    array_1d = np.arange(0.0, 1, 0.01, dtype=np.float32)  # 0から199までの配列を作成
    array_2d = np.random.choice(array_1d, (rows, cols), replace=True)  # ランダムに選択
    # array_2d[:,cols // 2:] = 0
    # array_2d = np.ones((rows, cols), dtype=np.float32)
    return array_2d

def mv_thread(a, b, c, i):
    c[i] = a.mv(b, i)

def main() -> None:
    np.set_printoptions(threshold=25)
    size = 1024
    n_threads = 4
    a = generate_2d_array(128*n_threads, size)
    b_list = np.arange(0.0, 1, 0.01, dtype=np.float32)
    b = np.random.choice(b_list, size, replace=True)
    sysinit(n_threads)
    a_subs = [None] * n_threads
    for i in range(n_threads):
        a_subs[i] = IMAXArray.from_numpy(a[i*128:(i+1)*128,])
    c_subs = [None] * n_threads
    c_threads = [None] * n_threads
    print("Iteration 1")
    for i in range(n_threads):
        c_threads[i] = threading.Thread(target=mv_thread, args=(a_subs[i], IMAXArray.from_numpy(b), c_subs, i))
        c_threads[i].start()
    for i in range(n_threads):
        c_threads[i].join()
    c = np.zeros(0, dtype=np.float32)
    for c_sub in c_subs:
        c = np.concatenate([c, IMAXArray.to_numpy(c_sub)])

    c_subs = [None] * n_threads
    c_threads = [None] * n_threads
    print("Iteration 2")
    for i in range(n_threads):
        c_threads[i] = threading.Thread(target=mv_thread, args=(a_subs[i], IMAXArray.from_numpy(b), c_subs, i))
        c_threads[i].start()
    for i in range(n_threads):
        c_threads[i].join()
    c = np.zeros(0, dtype=np.float32)
    for c_sub in c_subs:
        c = np.concatenate([c, IMAXArray.to_numpy(c_sub)])
    print("c_imax: ", c)

    time_init = datetime.datetime.now()
    c_numpy = a.dot(b)
    time_delta = datetime.datetime.now() - time_init
    print("c_numpy: ", c_numpy)
    delta = c - c_numpy
    print("sum(c_imax - c_numpy)", np.sum(delta, dtype=np.float32))
    print("numpy: ", time_delta.microseconds, "msec")

if __name__ == "__main__":
    main()