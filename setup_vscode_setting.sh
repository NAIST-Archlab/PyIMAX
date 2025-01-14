#!/bin/bash
mkdir ./.vscode
touch ./.vscode/settings.json
echo "{" > ./.vscode/settings.json
echo "    \"C_Cpp.default.includePath\": [" >> ./.vscode/settings.json
for include_path in `(python -m pybind11 --includes)`
do
    echo "        \"${include_path:2}\"," >> ./.vscode/settings.json
done
echo "    ]," >> ./.vscode/settings.json
echo "}" >> ./.vscode/settings.json
