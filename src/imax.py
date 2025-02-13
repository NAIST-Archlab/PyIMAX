from pyimax import IMAXArray, sysinit
import numpy as np


def generate_2d_array(rows, cols):
    array_1d = np.arange(200, dtype=np.float32)  # 0から199までの配列を作成
    array_2d = np.random.choice(array_1d, (rows, cols), replace=True)  # ランダムに選択
    return array_2d

def main() -> None:
    size = 100
    a = generate_2d_array(8, size)
    b = np.array(range(size), dtype=np.float32)
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
    print(a.dot(b))

if __name__ == "__main__":
    main()