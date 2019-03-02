import typing

from PySide2.QtWidgets import QApplication, QWidget

from python.platform import NodeStorage
from python.ui.grapheditor import GraphEditor


class UiManager:
    _node_storage: NodeStorage
    _root_widgets: typing.List[QWidget]

    def __init__(self, node_storage: NodeStorage):
        self._node_storage = node_storage
        node_storage.subscribe(lambda: self._on_state_update())

        self.app = QApplication([])

        self._root_widgets = []

        widget = GraphEditor(
            lambda: self._node_storage.state(),
            lambda action: self._node_storage.dispatch(action))

        widget.show()

        self._root_widgets.append(widget)
        self.app.exec_()

    def _on_state_update(self):
        for widget in self._root_widgets:
            widget.update()
