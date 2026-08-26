import zipfile

import pandas as pd
from docling_nlp.andromeda_doclang import DocLangXDocument, DocLangXNlp


def create_dclx(path):
    xml = '<doclang version="0.7"><text>FeSe is a material.</text></doclang>'

    with zipfile.ZipFile(path, "w") as archive:
        archive.writestr("[Content_Types].xml", "<Types></Types>")
        archive.writestr("_rels/.rels", "<Relationships></Relationships>")
        archive.writestr("document.xml", xml)
        archive.writestr(
            "annotations/properties.csv",
            "type,subj_hash,subj_name,subj_path,label,confidence\n"
            "language,123,text,/doclang[1]/text[1],en,0.99\n",
        )
        archive.writestr(
            "annotations/instances.csv",
            "type,subtype,subj_hash,subj_name,subj_path,conf,hash,ihash,coor_i,coor_j,char_i,char_j,ctok_i,ctok_j,wtok_i,wtok_j,wtok-match,name,original\n"
            "term,,123,text,/doclang[1]/text[1],1,456,789,,,0,4,0,1,0,1,true,FeSe,FeSe\n",
        )
        archive.writestr(
            "annotations/relations.csv",
            "flvr,name,conf,hash_i,hash_j,name_i,name_j\n"
            "42,contains,0.75,456,789,FeSe,material\n",
        )


def test_doclangx_document_read_query_write(tmp_path):
    input_path = tmp_path / "input.dclx"
    output_path = tmp_path / "output.dclx"
    create_dclx(input_path)

    doc = DocLangXDocument()
    assert doc.read(str(input_path))
    assert doc.valid()
    assert doc.has_archive()
    assert doc.has_annotations()

    summary = doc.summary()
    assert summary["properties"] == 1
    assert summary["instances"] == 1
    assert summary["relations"] == 1

    assert doc.annotation_paths() == [
        "annotations/properties.csv",
        "annotations/instances.csv",
        "annotations/relations.csv",
    ]
    assert "document.xml" in doc.archive_paths()

    properties = doc.properties()
    entities = doc.entities()
    instances = doc.instances()
    relations = doc.relations()

    assert isinstance(properties, pd.DataFrame)
    assert isinstance(entities, pd.DataFrame)
    assert isinstance(instances, pd.DataFrame)
    assert isinstance(relations, pd.DataFrame)
    assert properties.loc[0, "label"] == "en"
    assert entities.loc[0, "name"] == "FeSe"
    assert instances.equals(entities)
    assert relations.loc[0, "name"] == "contains"
    assert str(properties.dtypes["type"]) == "string"
    assert str(properties.dtypes["subj_hash"]) == "UInt64"
    assert str(properties.dtypes["confidence"]) == "Float32"
    assert str(entities.dtypes["hash"]) == "UInt64"
    assert str(entities.dtypes["coor_i"]) == "UInt64"
    assert str(entities.dtypes["wtok-match"]) == "boolean"
    assert pd.isna(entities.loc[0, "coor_i"])
    assert str(relations.dtypes["flvr"]) == "UInt64"
    assert str(relations.dtypes["conf"]) == "Float32"

    assert len(doc.query_properties(type="language")) == 1
    assert len(doc.query_entities(type="term", name_contains="Fe")) == 1
    assert len(doc.query_instances(type="term", name_contains="Fe")) == 1
    assert len(doc.query_instances(type="term", name="missing")) == 0
    assert len(doc.query_relations(name="contains", name_contains="material")) == 1
    assert len(doc.query_relations(name="contains", min_conf=0.9)) == 0

    assert doc.write(str(output_path))

    restored = DocLangXDocument()
    assert restored.read(str(output_path))
    assert restored.summary()["instances"] == 1
    assert len(restored.query_instances(name="FeSe")) == 1


def test_doclangx_document_apply_nlp_empty_model_expr(tmp_path):
    output_path = tmp_path / "empty-models.dclx"

    doc = DocLangXDocument()
    assert doc.read_xml('<doclang version="0.7"><text>Body text</text></doclang>')
    assert doc.apply_nlp("", progress_every=0)
    assert doc.summary()["instances"] == 0
    assert doc.write(str(output_path))

    restored = DocLangXDocument()
    assert restored.read(str(output_path))
    assert restored.valid()


def test_doclangx_nlp_reuses_initialised_models_across_documents():
    nlp = DocLangXNlp("")
    assert nlp.initialised()
    assert nlp.model_expr() == ""
    assert nlp.models() == []

    first = DocLangXDocument()
    second = DocLangXDocument()
    assert first.read_xml('<doclang version="0.7"><text>First text</text></doclang>')
    assert second.read_xml('<doclang version="0.7"><text>Second text</text></doclang>')

    assert nlp.apply(first, progress_every=0)
    assert nlp.apply(second, progress_every=0)
    assert first.summary()["instances"] == 0
    assert second.summary()["instances"] == 0


def test_doclangx_nlp_reports_uninitialised_apply():
    nlp = DocLangXNlp()
    doc = DocLangXDocument()
    assert doc.read_xml('<doclang version="0.7"><text>Body text</text></doclang>')

    assert not nlp.apply(doc, progress_every=0)
    assert nlp.last_error() == "models have not been initialised"
    assert doc.last_error() == "models have not been initialised"
