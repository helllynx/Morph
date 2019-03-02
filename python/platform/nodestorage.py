import typing
import copy

from python.base import Store
from .node import Node


class NodeFactory:
    _node_meta: typing.Dict[str, typing.GenericMeta]

    def __init__(self):
        self._node_meta = {}

    def register_node(self, node_model: str, meta_object: typing.GenericMeta) -> None:
        self._node_meta[node_model] = meta_object

    def create_node(self, node_model: str, **kwargs) -> Node:
        return self._node_meta[node_model](**kwargs)


class NodeStorage(Store):
    _nodes: typing.Dict[int, Node]
    _subscribers: typing.List[typing.Callable]
    _node_factory: NodeFactory

    def __init__(self):
        self._nodes = {}
        self._subscribers = []
        self._node_factory = NodeFactory()

    def register_node_model(self, node_model: str, meta_object: typing.GenericMeta) -> None:
        self._node_factory.register_node(node_model, meta_object)

    def dispatch(self, action: typing.Dict) -> None:
        if action['Type'] == 'CreateNode':
            self._create_node(action)
        elif action['Type'] == 'UpdateNodeMetadata':
            self._update_node_metadata(action)

        self._notify_subscribers()

    def subscribe(self, callback: typing.Callable) -> None:
        self._subscribers.append(callback)

    def state(self):
        return copy.deepcopy(self._nodes)

    def _notify_subscribers(self):
        for cb in self._subscribers:
            cb()

    def _create_node(self, action: typing.Dict):
        node = self._node_factory.create_node(action['NodeModel'], metadata=action['Metadata'])
        self._nodes[action['NodeId']] = node

    def _update_node_metadata(self, action: typing.Dict):
        self._nodes[action['NodeId']].update_metadata(action['Metadata'])
