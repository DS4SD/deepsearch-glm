
import os
import subprocess

def run(cmd, cwd="./"):

    print(f"\nlaunch: {cmd}")
    
    parts = cmd.split(" ")
    message = subprocess.run(parts, cwd=cwd)    

    if "returncode=0" in str(message):
        print(f" -> SUCCESS")
        return True

    print(f" -> ERROR with message: '{message}'\n")        
    return False
    
def build(setup_kwargs=None):

    cmds = [
        ["cmake -B ./build", os.path.abspath("./")],
        ["make install -j", os.path.abspath("./build")]
    ]

    for cmd in cmds:
        if not run(cmd[0], cwd=cmd[1]):
            break

def load(setup_kwargs=None):

    lang_source = "https://dl.fbaipublicfiles.com/fasttext/supervised-models/lid.176.bin"
    lang_target = os.path.abspath("./resources/models/fasttext/language/lid.176.bin")

    pos_dir = os.path.abspath("./resources/models/crf/part-of-speech/")
    pos_source = "https://www.logos.ic.i.u-tokyo.ac.jp/~tsuruoka/lapos/lapos-0.1.2.tar.gz"
    pos_target = f"{pos_dir}/lapos-0.1.2.tar.gz"
    
    cmds = [
        # language classifier
        #[f"curl {lang_source} -o {lang_target}", os.path.abspath("./")],
        [f"curl {lang_source} -o {lang_target} -s", os.path.abspath("./")],
        
        # POS CRF
        [f"curl {pos_source} -o {pos_target} -s", os.path.abspath("./")],
        [f"tar -xf {pos_target}", pos_dir],
        [f"cp ./lapos-0.1.2/model_wsj00-18/model.la ./en/model_wsj00-18/model.la", pos_dir],
        [f"cp ./lapos-0.1.2/model_wsj02-21/model.la ./en/model_wsj02-21/model.la", pos_dir],
        #[f"tree {pos_dir}", pos_dir],
        #[f"ls -l lapos-0.1.2", pos_dir],
        [f"rm -rf lapos-0.1.2", pos_dir],
        #[f"ls -l lapos-0.1.2.tar.gz", pos_dir],
        [f"rm lapos-0.1.2.tar.gz", pos_dir],

    ]

    for cmd in cmds:
        if not run(cmd[0], cwd=cmd[1]):
            break        
        
if "__main__"==__name__:
    build()

    load()
