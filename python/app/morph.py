from python.ui import UiManager
from python.platform import NodeStorage, Node, Knob


class DummyKnob(Knob):
    def __init__(self, metadata):
        super().__init__(metadata)

    @property
    def data(self):
        return None

    @property
    def type(self):
        return 'DummyKnob'


class DummyNode(Node):
    def __init__(self, metadata):
        super().__init__(metadata)
        self.add_input_knob('DummyInput1', DummyKnob({'Name': 'Dummy input 1'}))
        self.add_input_knob('DummyInput2', DummyKnob({'Name': 'Dummy input 2'}))
        self.add_output_knob('DummyOutput', DummyKnob({'Name': 'Dummy output'}))

    def compute(self):
        pass


node_storage = NodeStorage()

node_storage.register_node_model('DummyNode', DummyNode)

node_storage.dispatch({
    'Type': 'CreateNode',
    'NodeModel': 'DummyNode',
    'NodeId': 0,
    'Metadata': {'Name': 'Арсений,', 'PositionX': 100, 'PositionY': 100}
})

node_storage.dispatch({
    'Type': 'CreateNode',
    'NodeModel': 'DummyNode',
    'NodeId': 1,
    'Metadata': {'Name': 'Блэт', 'PositionX': 100, 'PositionY': 100}
})

node_storage.dispatch({
    'Type': 'CreateNode',
    'NodeModel': 'DummyNode',
    'NodeId': 2,
    'Metadata': {'Name': 'Я тут', 'PositionX': 100, 'PositionY': 100}
})

node_storage.dispatch({
    'Type': 'CreateNode',
    'NodeModel': 'DummyNode',
    'NodeId': 3,
    'Metadata': {'Name': 'Серьезный софт пилю', 'PositionX': 100, 'PositionY': 100}
})

ui_manager = UiManager(node_storage)
