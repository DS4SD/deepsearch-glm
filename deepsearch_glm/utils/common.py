import os


def get_scratch_dir():
    """Get scratch directory from environment variable `DEEPSEARCH_GLM_SCRATCH_DIR` (defined in .env)"""

    tmpdir = os.path.abspath(os.getenv("DEEPSEARCH_GLM_SCRATCH_DIR"))

    if not os.path.exists(tmpdir):
        os.mkdir(tmpdir)

    return tmpdir
