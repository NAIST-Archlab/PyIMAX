from pyimax import IMAXArray, sysinit
import numpy as np
import datetime


def generate_2d_array(rows, cols):
    # array_1d = np.arange(200, dtype=np.float32)  # 0から199までの配列を作成
    # array_2d = np.random.choice(array_1d, (rows, cols), replace=True)  # ランダムに選択
    # array_2d[:,cols // 2:] = 0
    array_2d = np.ones((rows, cols), dtype=np.float32)
    return array_2d

def main() -> None:
    size = 56*20
    # b = np.array(range(size), dtype=np.float32)
    a = generate_2d_array(256, size)
    b = np.ones((size), dtype=np.float32)
    print(a)
    print(b)
    sysinit(4)
    a_i = IMAXArray.from_numpy(a)
    b_i = IMAXArray.from_numpy(b)
    print(a_i)
    c_i = a_i.mv(b_i)
    print(a_i)
    print(b_i)
    print(c_i)
    b_i = IMAXArray.from_numpy(b)
    c_i = a_i.mv(b_i)
    print(b_i)
    print(c_i)
    b_i = IMAXArray.from_numpy(b)
    c_i = a_i.mv(b_i)
    print(b_i)
    print(c_i)
    b_i = IMAXArray.from_numpy(b)
    c_i = a_i.mv(b_i)
    print(IMAXArray.to_numpy(c_i))
    time_init = datetime.datetime.now()
    c = a.dot(b)
    time_delta = datetime.datetime.now() - time_init
    print(c)
    print("numpy: ", time_delta.microseconds, "msec")

if __name__ == "__main__":
    main()