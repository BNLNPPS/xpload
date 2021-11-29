find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xpl"
    COMMAND ${Python3_EXECUTABLE} -m venv venv
      && source venv/bin/activate
      && python -m pip install --upgrade pip
      && python -m pip install pyinstaller -r "${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt"
      && pyinstaller --onefile -n xpl --distpath . "${CMAKE_CURRENT_SOURCE_DIR}/xpload.py"
      && deactivate
)

add_custom_target(xpl ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xpl")

install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/xpl" TYPE BIN)
