#!/usr/bin/env python
"""Visualize term networks from annotated DocLang archives."""

from __future__ import annotations

import argparse
import json
import sys
from itertools import combinations
from pathlib import Path
from typing import Any

import pandas as pd
from docling_nlp.andromeda_doclang import DocLangXDocument

DEFAULT_MODELS = "language;term"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build and visualize a term network from one or more .dclx files."
    )
    parser.add_argument("inputs", nargs="+", type=Path, help="Input .dclx files")
    parser.add_argument(
        "--models",
        default=DEFAULT_MODELS,
        help=f"NLP models used when a document has no annotations. Default: {DEFAULT_MODELS}",
    )
    parser.add_argument(
        "--min-count",
        type=int,
        default=1,
        help="Minimum collapsed entity count for term nodes. Default: 1",
    )
    parser.add_argument(
        "--top-n",
        type=int,
        default=0,
        help="Keep only the top N terms by count. Default: keep all",
    )
    parser.add_argument(
        "--include-derived",
        action="store_true",
        help="Include derived suffix-tree entities as nodes.",
    )
    parser.add_argument(
        "--no-relations",
        action="store_true",
        help="Do not add edges from annotation relations.",
    )
    parser.add_argument(
        "--no-cooccurrence",
        action="store_true",
        help="Do not add term co-occurrence edges within the same DocLang path.",
    )
    parser.add_argument(
        "--write-annotated",
        action="store_true",
        help="Persist lazily annotated documents back to their input .dclx paths.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Write the NetworkX node-link graph JSON to this path.",
    )
    parser.add_argument(
        "--no-browser",
        action="store_true",
        help="Build/export the graph without opening netwulf.",
    )
    return parser.parse_args()


def require_graph_libraries(show_browser: bool) -> Any:
    try:
        import networkx as nx
    except ImportError as exc:
        raise SystemExit(
            "Missing optional dependency 'networkx'. Install visualization dependencies "
            "with: uv add networkx netwulf"
        ) from exc

    if show_browser:
        try:
            import netwulf
        except ImportError as exc:
            if exc.name == "netwulf":
                raise SystemExit(
                    "Missing optional dependency 'netwulf'. Install visualization "
                    "dependencies with: uv sync --extra networks"
                ) from exc
            raise SystemExit(
                "Could not import 'netwulf' because one of its dependencies is "
                f"missing: {exc}. Try: uv sync --extra networks"
            ) from exc

    return nx


def read_document(path: Path, models: str, write_annotated: bool) -> DocLangXDocument:
    doc = DocLangXDocument()
    if not doc.read(str(path)):
        raise RuntimeError(f"could not read {path}: {doc.last_error()}")

    if not doc.has_annotations():
        if not doc.apply_nlp(models, progress_every=0):
            raise RuntimeError(f"could not annotate {path}: {doc.last_error()}")
        if write_annotated and not doc.write(str(path)):
            raise RuntimeError(f"could not write annotated {path}: {doc.last_error()}")

    return doc


def term_entities(doc: DocLangXDocument, min_count: int, include_derived: bool) -> pd.DataFrame:
    entities = doc.query_entities(type="term", min_count=min_count)
    if not include_derived and "entity_kind" in entities:
        entities = entities[entities["entity_kind"] == "exact"]
    return entities.copy()


def top_entity_hashes(entities: pd.DataFrame, top_n: int) -> set[int] | None:
    if top_n <= 0 or entities.empty:
        return None

    top = entities.sort_values(["count", "name"], ascending=[False, True]).head(top_n)
    return {int(value) for value in top["hash"].tolist()}


def add_entity_nodes(graph: Any, entities: pd.DataFrame, doc_path: Path) -> set[int]:
    hashes: set[int] = set()

    for row in entities.itertuples(index=False):
        entity_hash = int(row.hash)
        hashes.add(entity_hash)

        if graph.has_node(entity_hash):
            graph.nodes[entity_hash]["count"] += int(row.count)
            graph.nodes[entity_hash]["documents"].add(str(doc_path))
            continue

        graph.add_node(
            entity_hash,
            label=str(row.name),
            name=str(row.name),
            type=str(row.type),
            subtype=str(row.subtype) if not pd.isna(row.subtype) else "",
            entity_kind=str(row.entity_kind),
            count=int(row.count),
            parent=str(row.parent) if not pd.isna(row.parent) else "",
            parent_hash=int(row.parent_hash) if not pd.isna(row.parent_hash) else 0,
            documents={str(doc_path)},
        )

    return hashes


def add_or_update_edge(graph: Any, source: int, target: int, kind: str, label: str) -> None:
    if source == target:
        return

    u, v = sorted((source, target))
    if graph.has_edge(u, v):
        graph.edges[u, v]["weight"] += 1
        graph.edges[u, v]["kinds"].add(kind)
        if label:
            graph.edges[u, v]["labels"].add(label)
        return

    graph.add_edge(u, v, weight=1, kinds={kind}, labels={label} if label else set())


def add_relation_edges(graph: Any, doc: DocLangXDocument, allowed_hashes: set[int]) -> None:
    relations = doc.relations()
    if relations.empty:
        return

    for row in relations.itertuples(index=False):
        source = DocLangXDocument.hash(str(row.name_i))
        target = DocLangXDocument.hash(str(row.name_j))
        if source not in allowed_hashes or target not in allowed_hashes:
            continue
        add_or_update_edge(graph, source, target, "relation", str(row.name))


def add_cooccurrence_edges(graph: Any, doc: DocLangXDocument, allowed_hashes: set[int]) -> None:
    instances = doc.query_instances(type="term")
    if instances.empty:
        return

    instances = instances[instances["hash"].isin(allowed_hashes)]
    for _, group in instances.groupby("subj_path", dropna=True):
        hashes = sorted({int(value) for value in group["hash"].tolist()})
        for source, target in combinations(hashes, 2):
            add_or_update_edge(graph, source, target, "cooccurrence", "cooccurs")


def normalize_graph_attributes(graph: Any) -> None:
    for _, data in graph.nodes(data=True):
        data["documents"] = sorted(data["documents"])

    for _, _, data in graph.edges(data=True):
        data["kinds"] = sorted(data["kinds"])
        data["labels"] = sorted(data["labels"])


def build_graph(args: argparse.Namespace) -> Any:
    nx = require_graph_libraries(show_browser=not args.no_browser)
    graph = nx.Graph()

    for path in args.inputs:
        doc = read_document(path, args.models, args.write_annotated)
        entities = term_entities(doc, args.min_count, args.include_derived)
        keep_hashes = top_entity_hashes(entities, args.top_n)
        if keep_hashes is not None:
            entities = entities[entities["hash"].isin(keep_hashes)]

        allowed_hashes = add_entity_nodes(graph, entities, path)
        if not args.no_relations:
            add_relation_edges(graph, doc, allowed_hashes)
        if not args.no_cooccurrence:
            add_cooccurrence_edges(graph, doc, allowed_hashes)

    normalize_graph_attributes(graph)
    return graph


def write_graph_json(graph: Any, output: Path) -> None:
    import networkx as nx

    data = nx.node_link_data(graph, edges="links")
    stringify_node_link_ids(data)
    output.write_text(json.dumps(data, indent=2), encoding="utf-8")


def stringify_node_link_ids(data: dict[str, Any]) -> None:
    for node in data.get("nodes", []):
        if "id" in node:
            node["id"] = str(node["id"])

    for link in data.get("links", []):
        if "source" in link:
            link["source"] = str(link["source"])
        if "target" in link:
            link["target"] = str(link["target"])


def visualize_graph(graph: Any) -> None:
    import networkx as nx
    import netwulf

    config = {
        "NodeLinkDiagram": {
            "nodeLabel": "label",
            "nodeSize": "count",
            "linkWidth": "weight",
        }
    }
    data = nx.node_link_data(graph, edges="links")
    stringify_node_link_ids(data)
    netwulf.visualize(data, config=config)


def main() -> int:
    args = parse_args()

    try:
        graph = build_graph(args)
        if args.output:
            write_graph_json(graph, args.output)
        if not args.no_browser:
            visualize_graph(graph)
    except Exception as exc:
        print(f"visualize.py: {exc}", file=sys.stderr)
        return 1

    print(
        f"built graph with {graph.number_of_nodes()} nodes and {graph.number_of_edges()} edges"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
