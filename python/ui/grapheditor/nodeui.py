import enum

from PySide2.QtCore import QRect, QPoint
from PySide2.QtGui import QPainter, QColor

from python.platform import Node


class NodeUiState(enum.Enum):
    Normal = 0
    Focused = 1
    Selected = 2


node_header_height = 50
node_knob_height = 40
node_width = 200

normal_color = QColor.fromRgb(50, 120, 30)
focused_color = QColor.fromRgb(50, 120, 60)
selected_color = QColor.fromRgb(60, 120, 130)


def draw_node(node: Node, node_state: NodeUiState, painter: QPainter):
    node_rect = QRect(node.metadata['PositionX'], node.metadata['PositionY'], node_width, node_header_height)

    if node_state == NodeUiState.Selected:
        color = selected_color
    elif node_state == NodeUiState.Focused:
        color = focused_color
    else:
        color = normal_color

    painter.fillRect(node_rect, color)
    painter.drawText(node_rect, node.metadata['Name'])

    for idx, (name, knob) in enumerate(node.input_knobs.items()):
        knob_rect = QRect(
            node_rect.x(),
            node_rect.y() + node_header_height + node_knob_height * idx,
            node_width,
            node_knob_height)

        painter.fillRect(knob_rect, color)
        painter.drawText(knob_rect, knob.metadata['Name'])

    input_knobs_count = len(node.input_knobs)

    for idx, (name, knob) in enumerate(node.output_knobs.items()):
        knob_rect = QRect(
            node_rect.x(),
            node_rect.y() + node_header_height + node_knob_height * (idx + input_knobs_count),
            node_width,
            node_knob_height)

        painter.fillRect(knob_rect, color)
        painter.drawText(knob_rect, knob.metadata['Name'])


def intersect_node(node: Node, pos: QPoint):
    node_rect = QRect(node.metadata['PositionX'], node.metadata['PositionY'], node_width, node_header_height)
    return node_rect.contains(pos)
