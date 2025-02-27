from pyimax import IMAXArray, sysinit
import numpy as np
import datetime
import threading


def generate_2d_array(rows, cols):
    # array_1d = np.arange(200, dtype=np.float32)  # 0から199までの配列を作成
    # array_2d = np.random.choice(array_1d, (rows, cols), replace=True)  # ランダムに選択
    # array_2d[:,cols // 2:] = 0
    array_2d = np.ones((rows, cols), dtype=np.float32)
    return array_2d

def mv_thread(a, b, c, i):
    c[i] = a.mv(b, i)

def main() -> None:
    size = 56*10
    # b = np.array(range(size), dtype=np.float32)
    a = generate_2d_array(256, size)
    b = np.ones((size), dtype=np.float32)
    b_i = IMAXArray.from_numpy(b)
    sysinit(4)
    a_subs = [None] * 4
    for i in range(4):
        a_subs[i] = IMAXArray.from_numpy(a[i*64:(i+1)*64,])
    print(a_subs)
    c_subs = [None] * 4
    c_threads = [None] * 4
    for i in range(4):
        c_threads[i] = threading.Thread(target=mv_thread, args=(a_subs[i], b_i, c_subs, i))
        c_threads[i].start()
    for i in range(4):
        c_threads[i].join()
    print(c_subs)
    for c_sub in c_subs:
        print(IMAXArray.to_numpy(c_sub))
    c_subs = [None] * 4
    c_threads = [None] * 4
    for i in range(4):
        c_threads[i] = threading.Thread(target=mv_thread, args=(a_subs[i], b_i, c_subs, i))
        c_threads[i].start()
    for i in range(4):
        c_threads[i].join()
    print(c_subs)
    for c_sub in c_subs:
        print(IMAXArray.to_numpy(c_sub))
    time_init = datetime.datetime.now()
    c = a.dot(b)
    time_delta = datetime.datetime.now() - time_init
    print(c)
    print("numpy: ", time_delta.microseconds, "msec")

if __name__ == "__main__":
    main()