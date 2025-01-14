from pyimax import imax
import numpy as np

def main() -> None:
    a = np.array([1, 2, 3, 4, 5], dtype=np.int32)
    imax.to_imax(a)

if __name__ == "__main__":
    main()