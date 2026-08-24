import os
import tempfile


def get_scratch_dir():
    """Get scratch directory from `DOCLING_NLP_SCRATCH_DIR` or a temp default."""

    tmpdir = os.getenv("DOCLING_NLP_SCRATCH_DIR")
    if tmpdir is None:
        tmpdir = os.path.join(tempfile.gettempdir(), "docling-nlp")

    tmpdir = os.path.abspath(tmpdir)

    if not os.path.exists(tmpdir):
        os.makedirs(tmpdir)

    return tmpdir
