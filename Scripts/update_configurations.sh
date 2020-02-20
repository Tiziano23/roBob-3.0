file=../Firmware/VSCode\ Configutations/c_cpp_properties.json

cp -u $file ../Firmware/Boards/Control\ Board/controller/.vscode/
cp -u $file ../Firmware/Boards/Sensors\ Board/line_comm_controller/.vscode/
cp -u $file ../Firmware/Boards/Sensors\ Board/color_detection_controller/.vscode/