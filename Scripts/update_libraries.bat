set ROOT=..\

xcopy %ROOT%\Firmware\Libraries\* "%ROOT%\Firmware\Boards\Control Board\controller\libraries" /S /Y
xcopy %ROOT%\Firmware\Libraries\* "%ROOT%\Firmware\Boards\Sensors Board\line_comm_controller\libraries" /S /Y
xcopy %ROOT%\Firmware\Libraries\* "%ROOT%\Firmware\Boards\Sensors Board\color_detection_controller\libraries" /S /Y