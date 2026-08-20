import json
from pathlib import Path

import pytest

import docling_nlp.utils.load_pretrained_models as loader

MODEL_FILES = {
    "language": (
        "fst_language.bin",
        "models/fasttext/language/fst_language.bin",
    ),
    "geoloc": ("rgx_geoloc.json", "models/rgx/geoloc/rgx_geoloc.json"),
}


def write_models_config(resources_dir: Path):
    (resources_dir / "models.json").write_text(
        json.dumps(
            {
                "huggingface": {
                    "repo-id": "docling-project/docling-nlp-models",
                    "revision": "main",
                },
                "nlp": {"trained-models": MODEL_FILES},
            }
        ),
        encoding="utf-8",
    )


def test_load_pretrained_models_downloads_to_configured_paths(tmp_path, monkeypatch):
    write_models_config(tmp_path)
    calls = []

    def fake_download(**kwargs):
        calls.append(kwargs)
        cached_file = tmp_path / f"cached-{len(calls)}"
        cached_file.write_bytes(kwargs["filename"].encode())
        return str(cached_file)

    monkeypatch.setattr(loader, "get_resources_dir", lambda: str(tmp_path))
    monkeypatch.setattr(loader, "hf_hub_download", fake_download)

    result = loader.load_pretrained_nlp_models()

    assert result == ["language", "geoloc"]
    assert (tmp_path / MODEL_FILES["language"][1]).read_bytes() == (
        MODEL_FILES["language"][1].encode()
    )
    assert calls[0]["repo_id"] == "docling-project/docling-nlp-models"
    assert calls[0]["revision"] == "main"
    assert [call["filename"] for call in calls] == [
        MODEL_FILES["language"][1],
        MODEL_FILES["geoloc"][1],
    ]


def test_load_pretrained_models_reuses_existing_files(tmp_path, monkeypatch):
    write_models_config(tmp_path)
    target = tmp_path / MODEL_FILES["language"][1]
    geoloc_target = tmp_path / MODEL_FILES["geoloc"][1]
    target.parent.mkdir(parents=True)
    geoloc_target.parent.mkdir(parents=True)
    target.write_bytes(b"existing")
    geoloc_target.write_bytes(b"existing")
    calls = []

    monkeypatch.setattr(loader, "get_resources_dir", lambda: str(tmp_path))
    monkeypatch.setattr(
        loader, "hf_hub_download", lambda **kwargs: calls.append(kwargs)
    )

    result = loader.load_pretrained_nlp_models()

    assert result == ["language", "geoloc"]
    assert calls == []
    assert target.read_bytes() == b"existing"


def test_load_pretrained_models_force_refreshes_existing_files(tmp_path, monkeypatch):
    write_models_config(tmp_path)
    target = tmp_path / MODEL_FILES["language"][1]
    target.parent.mkdir(parents=True)
    target.write_bytes(b"existing")

    def fake_download(**kwargs):
        cached_file = tmp_path / "cached"
        cached_file.write_bytes(b"refreshed")
        assert kwargs["force_download"] is True
        return str(cached_file)

    monkeypatch.setattr(loader, "get_resources_dir", lambda: str(tmp_path))
    monkeypatch.setattr(loader, "hf_hub_download", fake_download)

    loader.load_pretrained_nlp_models(force=True)

    assert target.read_bytes() == b"refreshed"


def test_load_pretrained_models_reports_download_failures(tmp_path, monkeypatch):
    write_models_config(tmp_path)
    monkeypatch.setattr(loader, "get_resources_dir", lambda: str(tmp_path))
    monkeypatch.setattr(
        loader,
        "hf_hub_download",
        lambda **kwargs: (_ for _ in ()).throw(OSError("no network")),
    )

    with pytest.raises(RuntimeError, match="Failed to download NLP model 'language'"):
        loader.load_pretrained_nlp_models()
