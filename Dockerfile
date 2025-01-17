FROM python:3.9

COPY ./pyimax /src/pyimax

COPY ./setup_vscode_setting.sh /src/pyimax

RUN pip install --upgrade pip && \
    pip install pybind11 && \
    cd /src/pyimax && \
    /bin/bash ./setup_vscode_setting.sh

RUN pip install /src/pyimax
RUN pip install numpy

WORKDIR /workspace/pyimax

CMD ["/bin/bash"]
