FROM python:3.9

WORKDIR /src/pyimax
COPY ./setup_vscode_setting.sh /src/pyimax

RUN pip install --upgrade pip && \
    pip install pybind11 && \
    cd /src/pyimax && \
    /bin/bash ./setup_vscode_setting.sh

RUN pip install numpy

COPY ./pyimax /src/pyimax
RUN pip install /src/pyimax

WORKDIR /workspace/pyimax

CMD ["/bin/bash"]
