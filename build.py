
import os
import subprocess

def run(cmd, cwd="./"):

    parts = cmd.split(" ")

    message = subprocess.run(parts, cwd=cwd)    
    if "returncode=0" in str(message):
        print(f"{cmd}: SUCCESS")
        return True

    print(f"{cmd}: ERROR with message '{message}'")        
    return False
    
def build(setup_kwargs=None):

    cmds = [
        ["cmake -B ./build", os.path.abspath("./")],
        ["make install -j", os.path.abspath("./build")]
    ]

    for cmd in cmds:
        if not run(cmd[0], cwd=cmd[1]):
            break
        
if "__main__"==__name__:
    build()
