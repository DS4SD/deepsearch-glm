import zipfile

from docling_nlp.andromeda_doclang import DocLangXDocument


def create_dclx(path):
    xml = "<doclang version=\"0.7\"><text>FeSe is a material.</text></doclang>"

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

    assert doc.properties()["data"][0][4] == "en"
    assert doc.instances()["data"][0][-2] == "FeSe"
    assert doc.relations()["data"][0][1] == "contains"

    assert len(doc.query_properties(type="language")["data"]) == 1
    assert len(doc.query_instances(type="term", name_contains="Fe")["data"]) == 1
    assert len(doc.query_instances(type="term", name="missing")["data"]) == 0
    assert len(doc.query_relations(name="contains", name_contains="material")["data"]) == 1
    assert len(doc.query_relations(name="contains", min_conf=0.9)["data"]) == 0

    assert doc.write(str(output_path))

    restored = DocLangXDocument()
    assert restored.read(str(output_path))
    assert restored.summary()["instances"] == 1
    assert len(restored.query_instances(name="FeSe")["data"]) == 1


def test_doclangx_document_apply_nlp_empty_model_expr(tmp_path):
    output_path = tmp_path / "empty-models.dclx"

    doc = DocLangXDocument()
    assert doc.read_xml("<doclang version=\"0.7\"><text>Body text</text></doclang>")
    assert doc.apply_nlp("", progress_every=0)
    assert doc.summary()["instances"] == 0
    assert doc.write(str(output_path))

    restored = DocLangXDocument()
    assert restored.read(str(output_path))
    assert restored.valid()
