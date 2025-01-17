from pyimax import IMAXArray, sysinit
import numpy as np

def main() -> None:
    a = np.array([1, 2, 3, 4, 5], dtype=np.int32)
    sysinit(8)
    a_i = IMAXArray.from_numpy(a)
    print(a_i)
    print(IMAXArray.to_numpy(a_i))

if __name__ == "__main__":
    main()