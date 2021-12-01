find_package(Python3 COMPONENTS Interpreter REQUIRED)

configure_file(xpload_config.py.in xpload_config.py @ONLY)
configure_file(xpload.py xpload.py COPYONLY)
configure_file(requirements.txt requirements.txt COPYONLY)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xpl"
    COMMAND ${Python3_EXECUTABLE} -m venv venv
      && source venv/bin/activate
      && python -m pip install --upgrade pip
      && python -m pip install pyinstaller -r requirements.txt
      && pyinstaller --onefile -n xpl --distpath . xpload.py
      && deactivate
)

add_custom_target(xpl ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xpl")

install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/xpl" TYPE BIN)
