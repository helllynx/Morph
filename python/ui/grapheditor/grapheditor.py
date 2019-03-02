import typing

from PySide2.QtCore import QSize, SIGNAL, QObject
from PySide2.QtGui import QPaintEvent, QPainter, QColor, QResizeEvent, QMouseEvent
from PySide2.QtWidgets import QWidget, QMenuBar, QMenu, QAction

from .nodeui import NodeUiState, draw_node, intersect_node


class GraphEditor(QWidget):
    _get_state_cb: typing.Callable
    _dispatch_cb: typing.Callable
    _menu_bar: QMenuBar

    _node_width: int = 120
    _node_height: int = 80

    def __init__(self, get_state_cb: typing.Callable, dispatch_cb: typing.Callable, parent: QWidget = None):
        super().__init__(parent)
        self.setMouseTracking(True)

        self._get_state_cb = get_state_cb
        self._dispatch_cb = dispatch_cb
        self._menu_bar = QMenuBar(self)

        self.create_node_action = QAction('Create node', self)
        QObject.connect(
            self.create_node_action,
            SIGNAL("triggered()"),
            lambda: self._dispatch_cb({
                'Type': 'CreateNode',
                'NodeModel': 'DummyNode',
                'NodeId': 1,
                'Metadata': {'Name': 'New dummy node', 'PositionX': 200, 'PositionY': 120}
            }))

        self.node_menu = QMenu('Node')
        self.node_menu.addAction(self.create_node_action)

        self._menu_bar.addMenu(self.node_menu)

        self._focused_node_id = None
        self._clicked_node_id = None
        self._selected_node_id = None
        self._last_click_pos = None

    def sizeHint(self):
        return QSize(800, 600)

    def resizeEvent(self, event: QResizeEvent):
        self._menu_bar.setGeometry(0, 0, event.size().width(), self._menu_bar.sizeHint().height())

    def paintEvent(self, event: QPaintEvent):
        if self._get_state_cb is None:
            print("GraphEditor: state callback is not specified. Ignoring paint event.")
            return

        state = self._get_state_cb()

        painter = QPainter(self)
        painter.fillRect(self.rect(), QColor.fromRgb(58, 58, 58))

        for node_id, node in state.items():
            if node_id == self._selected_node_id:
                state = NodeUiState.Selected
            elif node_id == self._focused_node_id:
                state = NodeUiState.Focused
            else:
                state = NodeUiState.Normal

            draw_node(node, state, painter)

    def mouseMoveEvent(self, event: QMouseEvent):
        state = self._get_state_cb()

        if self._clicked_node_id is not None and self._clicked_node_id in state.keys():
            self._handle_node_grab(event, state)
        else:
            self._handle_mouse_move(event, state)

    def _handle_mouse_move(self, event, state):
        prev_focused_node_id = self._focused_node_id
        self._focused_node_id = None
        for node_id, node in state.items():
            if intersect_node(node, event.pos()):
                self._focused_node_id = node_id
        if self._focused_node_id != prev_focused_node_id:
            self.update()

    def _handle_node_grab(self, event, state):
        drag_delta = event.pos() - self._last_click_pos
        new_x = state[self._clicked_node_id].metadata['PositionX'] + drag_delta.x()
        new_y = state[self._clicked_node_id].metadata['PositionY'] + drag_delta.y()

        self._last_click_pos = event.pos()
        self._dispatch_cb({
            'Type': 'UpdateNodeMetadata',
            'NodeId': self._clicked_node_id,
            'Metadata': {'PositionX': new_x, 'PositionY': new_y}
        })

    def mousePressEvent(self, event: QMouseEvent):
        state = self._get_state_cb()

        for node_id, node in state.items():
            if intersect_node(node, event.pos()):
                self._clicked_node_id = node_id
                self._last_click_pos = event.pos()

    def mouseReleaseEvent(self, event: QMouseEvent):
        self._selected_node_id = self._clicked_node_id
        self._clicked_node_id = None
        self._last_click_pos = None
        self.update()
