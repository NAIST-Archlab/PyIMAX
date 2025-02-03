from pyimax import IMAXArray, sysinit
import numpy as np

def main() -> None:
    size = 512
    a = np.array([range(size), range(size), range(size), range(size), range(size), range(size), range(size), range(size), range(size), range(size)], dtype=np.float32)
    b = np.array(range(size), dtype=np.float32)
    sysinit(8)
    a_i = IMAXArray.from_numpy(a)
    b_i = IMAXArray.from_numpy(b)
    print(a_i)
    c_i = a_i.mv(b_i)
    print(a_i)
    print(b_i)
    print(c_i)
    print(IMAXArray.to_numpy(c_i))
    print(a.dot(b))

if __name__ == "__main__":
    main()