set root=..\
xcopy %root%\Firmware\Libraries\* "%root%\Firmware\Boards\Control Board\controller\libraries" /s /y /exclude:.\exclude.txt
xcopy %root%\Firmware\Libraries\* "%root%\Firmware\Boards\Sensors Board\line_comm_controller\libraries" /s /y /exclude:.\exclude.txt
xcopy %root%\Firmware\Libraries\* "%root%\Firmware\Boards\Sensors Board\color_detection_controller\libraries" /s /y /exclude:.\exclude.txt