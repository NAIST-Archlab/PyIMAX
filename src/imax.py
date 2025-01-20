from pyimax import IMAXArray, sysinit
import numpy as np

def main() -> None:
    a = np.array([[1, 2, 3, 4, 5], [6, 7, 8, 9, 10]], dtype=np.float32)
    b = np.array([1, 2, 3, 4, 5], dtype=np.float32)
    sysinit(8)
    a_i = IMAXArray.from_numpy(a)
    b_i = IMAXArray.from_numpy(b)
    print(a_i)
    c_i = a_i.mv(b_i)
    print(c_i)
    print(IMAXArray.to_numpy(c_i))

if __name__ == "__main__":
    main()